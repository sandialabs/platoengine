#include "plato/filter/extension/kernel_filters/SphericalMaskUtilities.hpp"

#include <Teuchos_ArrayViewDecl.hpp>
#include <Teuchos_EReductionType.hpp>
#include <algorithm>
#include <boost/math/constants/constants.hpp>
#include <boost/mpi/collectives.hpp>
#include <boost/mpi/collectives/broadcast.hpp>
#include <boost/mpi/communicator.hpp>
#include <cmath>
#include <iterator>
#include <ranges>
#include <utility>

#include "plato/filter/extension/kernel_filters/LinearMaskBuilderUtilities.hpp"
#include "plato/mesh/EntityCounts.hpp"
#include "plato/mesh/Mesh.hpp"
#include "plato/mesh/MeshQuantities.hpp"

namespace plato::filter::extension::kernel_filters
{

auto distribute_search_vectors_and_stk_search_with_spheres(const TargetRowVector& aTargetRowVector,
                                                           const SourceColumnVector& aSourceColumnVector,
                                                           const double aSearchRadius,
                                                           const boost::mpi::communicator& aCommunicator)
    -> third_party_integration::stk_search::SearchResults
{
    namespace tpi = third_party_integration;
    const auto tLocalSearchPointWithIdentifiers =
        detail::distribute_and_label_search_vector(aSourceColumnVector, aCommunicator);

    const tpi::tpetra::TpetraMultiVector tLocalRowCenterCoordinates =
        detail::create_tpetravector_coordinates(aTargetRowVector.mValue, aCommunicator);
    const auto tLocalSearchSpheresWithIdentifiers =
        detail::stk_search_spheres(tLocalRowCenterCoordinates, SearchRadius{aSearchRadius}, aCommunicator.rank());

    return tpi::stk_search::stk_search(tLocalSearchSpheresWithIdentifiers, tLocalSearchPointWithIdentifiers,
                                       aCommunicator);
}

namespace detail
{

double linear_ramp_weight(const Distance aDistance, const SearchRadius aSearchRadius)
{
    return std::max(0.0, 1.0 - aDistance.mValue / aSearchRadius.mValue);
}

double filter_volume(const SearchRadius aFilterRadius)
{
    return 4.0 / 3.0 * boost::math::constants::pi<double>() * aFilterRadius.mValue * aFilterRadius.mValue *
           aFilterRadius.mValue;
}

double filter_area(const SearchRadius aFilterRadius)
{
    return boost::math::constants::pi<double>() * aFilterRadius.mValue * aFilterRadius.mValue;
}

auto stk_search_spheres(const third_party_integration::tpetra::TpetraMultiVector& aCenteringCoordinates,
                        const SearchRadius aRadius,
                        const int aRank) -> std::vector<third_party_integration::stk_search::SearchSphereWithIdentifier>
{
    namespace tpi = third_party_integration;
    const auto tNumberOfLocalElements = tpi::tpetra::number_of_local_elements(aCenteringCoordinates.getMap());
    auto tLocalSearchSpheresWithIdentifier =
        std::vector<tpi::stk_search::SearchSphereWithIdentifier>(tNumberOfLocalElements);

    for (const auto tLocalIndex : std::views::iota(0, tNumberOfLocalElements))
    {
        const auto tGlobalID = aCenteringCoordinates.getMap()->getGlobalElement(tLocalIndex);
        const tpi::common::Coordinate tCoordinate =
            tpi::tpetra::multivector_coordinate(aCenteringCoordinates, tLocalIndex);
        tLocalSearchSpheresWithIdentifier[tLocalIndex] = tpi::stk_search::SearchSphereWithIdentifier(
            {tpi::stk_search::create_sphere(tCoordinate, tpi::stk_search::STKRadius{aRadius.mValue}),
             tpi::stk_search::Identifier{tGlobalID, aRank}});
    }
    return tLocalSearchSpheresWithIdentifier;
}

auto average_nodes_in_filter_radius_estimate(const mesh::Mesh& aMesh, const SearchRadius aFilterRadius) -> int
{
    const double tSearchVolume =
        mesh::EntityCounts{aMesh}.is2D() ? filter_area(aFilterRadius) : filter_volume(aFilterRadius);
    const auto tAverageNodalDensity = mesh::MeshQuantities{aMesh}.averageNodalDensity();

    return static_cast<int>(tAverageNodalDensity * tSearchVolume);
}

}  // namespace detail

auto distribute_search_vectors_and_stk_search_with_spheres(const TargetRowVector& aTargetRowVector,
                                                           const SourceColumnVector& aSourceColumnVector,
                                                           const double aRadius,
                                                           const boost::mpi::communicator& aCommunicator,
                                                           const Wedge& aWedge)
    -> third_party_integration::stk_search::SearchResults
{
    namespace tpi = third_party_integration;
    const auto tLocalSearchPointWithIdentifiers =
        detail::distribute_and_label_search_vector(aSourceColumnVector, aCommunicator);

    const auto tReflectedSearchPointsWithIdentifiers =
        detail::reflect_points_in_mirrored_wedge<third_party_integration::stk_search::SearchPointWithIdentifier>(
            tLocalSearchPointWithIdentifiers, aWedge);

    const tpi::tpetra::TpetraMultiVector tLocalRowCenterCoordinates =
        detail::create_tpetravector_coordinates(aTargetRowVector.mValue, aCommunicator);
    const auto tLocalSearchSpheresWithIdentifiers =
        detail::stk_search_spheres(tLocalRowCenterCoordinates, SearchRadius{aRadius}, aCommunicator.rank());

    return tpi::stk_search::stk_search(tLocalSearchSpheresWithIdentifiers, tReflectedSearchPointsWithIdentifiers,
                                       aCommunicator);
}

}  // namespace plato::filter::extension::kernel_filters
