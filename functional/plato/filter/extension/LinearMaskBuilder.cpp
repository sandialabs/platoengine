#include "plato/filter/extension/LinearMaskBuilder.hpp"

#include <Teuchos_ArrayViewDecl.hpp>
#include <Teuchos_EReductionType.hpp>
#include <boost/math/constants/constants.hpp>
#include <boost/mpi/collectives.hpp>

#include "plato/mesh/EntityCounts.hpp"
#include "plato/mesh/EntityRetrieval.hpp"
#include "plato/mesh/Mesh.hpp"
#include "plato/mesh/MeshQuantities.hpp"
#include "plato/utilities/IndexRange.hpp"

namespace plato::filter::extension
{

namespace
{
constexpr bool kZeroOut = true;

auto center_coordinates(const mesh::Mesh& aMesh, input_parser::KernelFilterCenteringTypes aCenteringType)
    -> std::vector<third_party_integration::common::Coordinate>
{
    if (aCenteringType == input_parser::KernelFilterCenteringTypes::kElementCentered)
    {
        return mesh::EntityRetrieval{aMesh}.designDomainElementCentroids();
    }
    else
    {
        return mesh::EntityRetrieval{aMesh}.designDomainNodalCoordinates();
    }
}

/// @brief create nodal coordinate tpetra container of  @a aNodalCoordinates
auto create_nodal_coordinates(const std::vector<third_party_integration::common::Coordinate>& aNodalCoordinates,
                              const boost::mpi::communicator& aCommunicator)
    -> third_party_integration::tpetra::TpetraMultiVector
{
    namespace tpi = third_party_integration;

    const auto tCommunicator = Teuchos::rcp(new Teuchos::MpiComm<int>(aCommunicator));
    auto tMap =
        Teuchos::rcp(new tpi::tpetra::TpetraMap(aNodalCoordinates.size(), tpi::tpetra::kIndexBase, tCommunicator));
    auto tNodalCoordinates = tpi::tpetra::TpetraMultiVector(tMap, tpi::tpetra::kNumberOfCartesianDimensions, kZeroOut);
    tpi::tpetra::distribute_on_tpetra_multivector(aNodalCoordinates, tNodalCoordinates);
    return tNodalCoordinates;
}

}  // namespace

LinearMaskBuilder::LinearMaskBuilder(const mesh::Mesh& aMesh,
                                     const input_parser::KernelFilterCenteringTypes aCenteringType,
                                     const SearchRadius aSearchRadius,
                                     const boost::mpi::communicator& aCommunicator)
    : mCommunicator(aCommunicator),
      mSearchRadius(aSearchRadius.mValue),
      mMaximumConnectivityEstimate(detail::maximum_connectivity_estimate(aMesh, aSearchRadius)),
      mRowCenterCoordinates(center_coordinates(aMesh, aCenteringType)),
      mNodalCoordinates(
          create_nodal_coordinates(mesh::EntityRetrieval{aMesh}.designDomainNodalCoordinates(), mCommunicator)),
      mLocalSearchPointWithIdentifiers(detail::stk_search_points(mNodalCoordinates, mCommunicator.rank()))
{
    generateDistanceMap();
}

LinearMaskBuilder::LinearMaskBuilder(const NodalVector& aNodalCoordinates,
                                     CenterVector aCenters,
                                     const SearchRadius aSearchRadius,
                                     const int aMaximumConnectivityEstimate,
                                     const boost::mpi::communicator& aCommunicator)
    : mCommunicator(aCommunicator),
      mSearchRadius(aSearchRadius.mValue),
      mMaximumConnectivityEstimate(aMaximumConnectivityEstimate),
      mRowCenterCoordinates(std::move(aCenters.mValue)),
      mNodalCoordinates(create_nodal_coordinates(aNodalCoordinates.mValue, mCommunicator)),
      mLocalSearchPointWithIdentifiers(detail::stk_search_points(mNodalCoordinates, mCommunicator.rank()))
{
    generateDistanceMap();
}

auto LinearMaskBuilder::mask() const -> const third_party_integration::tpetra::TpetraCRSMatrix& { return *mLinearMask; }

auto LinearMaskBuilder::generateRow(third_party_integration::common::Coordinate aCenter)
    -> std::pair<TpetraGlobalOrdinalVector, TpetraScalarVector>
{
    namespace tpi = third_party_integration;

    const auto tSearchResults =
        tpi::stk_search::find_points_in_sphere(aCenter, mSearchRadius, mLocalSearchPointWithIdentifiers, mCommunicator);

    auto tRow = tpi::tpetra::TpetraVector(mNodalCoordinates.getMap(), kZeroOut);

    double tSum = 0;
    for (const auto tLocalIndex :
         utilities::IndexRange{tpi::tpetra::number_of_local_elements(mNodalCoordinates.getMap())})
    {
        const tpi::stk_search::Identifier tLocalIdentifier{tLocalIndex, mCommunicator.rank()};
        if (tpi::stk_search::is_in_search_results(tLocalIdentifier, tSearchResults))
        {
            const tpi::common::Coordinate tLocalCoordinate =
                tpi::tpetra::multivector_coordinate(mNodalCoordinates, tLocalIndex);

            const double tDistance = tpi::common::magnitude(aCenter - tLocalCoordinate);
            const double tWeight{detail::linear_ramp_weight(Distance{tDistance}, SearchRadius{mSearchRadius})};

            const auto tGlobalID = mNodalCoordinates.getMap()->getGlobalElement(tLocalIndex);
            tRow.replaceGlobalValue(tGlobalID, tWeight);

            tSum += tWeight;
        }
    }
    double tGlobalSumOfWeights;
    const auto tCommunicator(Teuchos::rcp(new Teuchos::MpiComm<int>(mCommunicator)));
    Teuchos::reduceAll<int, double>(*tCommunicator, Teuchos::REDUCE_SUM, tSum, Teuchos::outArg(tGlobalSumOfWeights));

    return detail::normalize_nonzero_weights(tRow, detail::RowSum{tGlobalSumOfWeights},
                                             detail::EstimatedConnectivity{mMaximumConnectivityEstimate});
}

void LinearMaskBuilder::generateDistanceMap()
{
    namespace tpi = third_party_integration;

    const auto tCommunicator = Teuchos::rcp(new Teuchos::MpiComm<int>(mCommunicator));
    auto tCrsRowMap =
        Teuchos::rcp(new tpi::tpetra::TpetraMap(mRowCenterCoordinates.size(), tpi::tpetra::kIndexBase, tCommunicator));
    auto tCrsDomainMap = Teuchos::rcp(
        new tpi::tpetra::TpetraMap(mNodalCoordinates.getGlobalLength(), tpi::tpetra::kIndexBase, tCommunicator));

    mLinearMask = Teuchos::RCP<tpi::tpetra::TpetraCRSMatrix>{
        new tpi::tpetra::TpetraCRSMatrix(tCrsRowMap, mMaximumConnectivityEstimate)};

    for (const auto tGlobalIndex :
         utilities::IndexRange{static_cast<tpi::tpetra::TpetraGlobalOrdinal>(mRowCenterCoordinates.size())})
    {
        const tpi::common::Coordinate tGlobalCoordinate = mRowCenterCoordinates[tGlobalIndex];
        auto [tGlobalNonZeroIndices, tGlobalNonZeroWeights] = generateRow(tGlobalCoordinate);

        mLinearMask->insertGlobalValues(tGlobalIndex,
                                        Teuchos::ArrayView<tpi::tpetra::TpetraGlobalOrdinal>(tGlobalNonZeroIndices),
                                        Teuchos::ArrayView<tpi::tpetra::TpetraScalar>(tGlobalNonZeroWeights));
    }
    mLinearMask->fillComplete(tCrsDomainMap, tCrsRowMap);
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

unsigned int maximum_connectivity_estimate(const mesh::Mesh& aMesh, const SearchRadius aFilterRadius)
{
    const double tAverageNodalDensity = mesh::MeshQuantities{aMesh}.averageNodalDensity();
    const double tSearchVolume =
        mesh::EntityCounts{aMesh}.is2D() ? filter_area(aFilterRadius) : filter_volume(aFilterRadius);
    return static_cast<int>(tSearchVolume * tAverageNodalDensity * kMaxMultiplier);
}

auto normalize_nonzero_weights(const third_party_integration::tpetra::TpetraVector& aRowVector,
                               const RowSum aRowSum,
                               const EstimatedConnectivity aEstimatedConnectivity)
    -> std::pair<TpetraGlobalOrdinalVector, TpetraScalarVector>
{
    namespace tpi = third_party_integration;

    auto tNonZeroWeightVector = std::vector<tpi::tpetra::TpetraScalar>{};
    auto tNonZeroGlobalIndices = std::vector<tpi::tpetra::TpetraGlobalOrdinal>{};
    tNonZeroWeightVector.reserve(aEstimatedConnectivity.mValue);
    tNonZeroGlobalIndices.reserve(aEstimatedConnectivity.mValue);

    for (const auto tLocalIndex : utilities::IndexRange{tpi::tpetra::number_of_local_elements(aRowVector.getMap())})
    {
        const tpi::tpetra::TpetraScalar tReplacementWeight = aRowVector.getData()[tLocalIndex] / aRowSum.mValue;
        if (tReplacementWeight > 0)
        {
            const auto tGlobalID = aRowVector.getMap()->getGlobalElement(tLocalIndex);
            tNonZeroGlobalIndices.push_back(tGlobalID);
            tNonZeroWeightVector.push_back(tReplacementWeight);
        }
    }
    return {tNonZeroGlobalIndices, tNonZeroWeightVector};
}

std::vector<third_party_integration::stk_search::SearchPointWithIdentifier> stk_search_points(
    const third_party_integration::tpetra::TpetraMultiVector& aNodalCoordinates, const int aRank)
{
    namespace tpi = third_party_integration;

    const auto tNumberOfLocalElements = tpi::tpetra::number_of_local_elements(aNodalCoordinates.getMap());
    auto tLocalSearchPointWithIdentifiers =
        std::vector<tpi::stk_search::SearchPointWithIdentifier>(tNumberOfLocalElements);

    for (const auto tLocalIndex : utilities::IndexRange{tNumberOfLocalElements})
    {
        const tpi::stk_search::Identifier tIdentifier{tLocalIndex, aRank};
        const tpi::common::Coordinate tCoordinate = tpi::tpetra::multivector_coordinate(aNodalCoordinates, tLocalIndex);
        tLocalSearchPointWithIdentifiers[tLocalIndex] =
            tpi::stk_search::SearchPointWithIdentifier({tpi::stk_search::convert_coordinate(tCoordinate), tIdentifier});
    }
    return tLocalSearchPointWithIdentifiers;
}

}  // namespace detail

}  // namespace plato::filter::extension
