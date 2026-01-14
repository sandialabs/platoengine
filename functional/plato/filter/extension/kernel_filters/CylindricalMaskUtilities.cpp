
#include "plato/filter/extension/kernel_filters/CylindricalMaskUtilities.hpp"

#include <boost/math/constants/constants.hpp>
#include <cmath>
#include <ranges>

#include "plato/mesh/EntityCounts.hpp"
#include "plato/mesh/EntityRetrieval.hpp"
#include "plato/mesh/MeshQuantities.hpp"
#include "plato/third_party_integration/common/Vector3.hpp"

namespace plato::filter::extension::kernel_filters
{

namespace detail
{

double cylinder_ramp_weight(const CylinderCenter& aCylinderCenter,
                            const CylinderAxis& aCylinderAxis,
                            const CylinderRadius aCylinderRadius,
                            const QueryLocation& aQueryLocation)
{
    namespace tpic = third_party_integration::common;
    const auto tDistance =
        tpic::magnitude(tpic::cross(aQueryLocation.mValue - aCylinderCenter.mValue, aCylinderAxis.mValue)) /
        tpic::magnitude(aCylinderAxis.mValue);

    return std::max(0.0, 1.0 - tDistance / aCylinderRadius.mValue);
}

auto stk_search_boxes(const third_party_integration::tpetra::TpetraMultiVector& aCenteringCoordinates,
                      const CylinderRadius aRadius,
                      const CylinderAxis& aAxis,
                      const int aRank) -> std::vector<third_party_integration::stk_search::SearchBoxWithIdentifier>
{
    namespace tpi = third_party_integration;
    const auto tNumberOfLocalElements = tpi::tpetra::number_of_local_elements(aCenteringCoordinates.getMap());
    auto tLocalSearchBoxesWithIdentifier =
        std::vector<tpi::stk_search::SearchBoxWithIdentifier>(tNumberOfLocalElements);

    for (const auto tLocalIndex : std::views::iota(0, tNumberOfLocalElements))
    {
        const auto tGlobalID = aCenteringCoordinates.getMap()->getGlobalElement(tLocalIndex);
        const tpi::common::Coordinate tCoordinate =
            tpi::tpetra::multivector_coordinate(aCenteringCoordinates, tLocalIndex);
        const auto tDimension =
            tpi::common::Coordinate{aRadius.mValue + aAxis.mValue.x, aRadius.mValue + aAxis.mValue.y,
                                    aRadius.mValue + aAxis.mValue.z} *
            2.0;
        tLocalSearchBoxesWithIdentifier[tLocalIndex] = tpi::stk_search::SearchBoxWithIdentifier(
            {tpi::stk_search::search_box(tpi::stk_search::STKBoxCenter{tCoordinate},
                                         tpi::stk_search::STKBoxDimension{tDimension}),
             tpi::stk_search::Identifier{tGlobalID, aRank}});
    }
    return tLocalSearchBoxesWithIdentifier;
}

auto distribute_search_vectors_and_stk_search_with_cylinders(const TargetRowVector& aTargetRowVector,
                                                             const SourceColumnVector& aSourceColumnVector,
                                                             const CylinderAxis& aCylinderAxis,
                                                             const CylinderRadius aCylinderRadius,
                                                             const boost::mpi::communicator& aCommunicator)
    -> third_party_integration::stk_search::SearchResults
{
    namespace tpi = third_party_integration;
    const auto tLocalSearchPointWithIdentifiers =
        detail::distribute_and_label_search_vector(aSourceColumnVector, aCommunicator);

    const tpi::tpetra::TpetraMultiVector tLocalRowCenterCoordinates =
        detail::create_tpetravector_coordinates(aTargetRowVector.mValue, aCommunicator);

    const auto tLocalSearchBoxWithIdentifiers =
        detail::stk_search_boxes(tLocalRowCenterCoordinates, aCylinderRadius, aCylinderAxis, aCommunicator.rank());

    return tpi::stk_search::stk_search<tpi::stk_search::SearchBoxWithIdentifier>(
        tLocalSearchBoxWithIdentifiers, tLocalSearchPointWithIdentifiers, aCommunicator);
}

auto cylindrical_polar_coordinates(const third_party_integration::common::Coordinate& aCoordinate)
    -> CylindricalPolarCoordinate
{
    const auto tRadialValue = std::sqrt(aCoordinate.x * aCoordinate.x + aCoordinate.y * aCoordinate.y);
    const auto tThetaValue = std::atan2(aCoordinate.y, aCoordinate.x);
    return CylindricalPolarCoordinate{
        third_party_integration::common::Coordinate{tRadialValue, tThetaValue, aCoordinate.z}};
}

auto cylindrical_polar_coordinates(const std::vector<third_party_integration::common::Coordinate>& aCoordinates)
    -> CylindricalPolarCoordinates
{
    std::vector<third_party_integration::common::Coordinate> tPolarCoordinates;
    tPolarCoordinates.reserve(aCoordinates.size());

    std::ranges::transform(aCoordinates, std::back_inserter(tPolarCoordinates),
                           [](const auto& aPoint) { return cylindrical_polar_coordinates(aPoint).mValue; });
    return CylindricalPolarCoordinates{tPolarCoordinates};
}

}  // namespace detail
}  // namespace plato::filter::extension::kernel_filters
