#include "plato/filter/extension/LinearMaskFactory.hpp"

#include <Teuchos_ArrayViewDecl.hpp>
#include <Teuchos_EReductionType.hpp>
#include <boost/mpi/collectives.hpp>
#include <set>

#include "plato/utilities/MultidimensionalRange.hpp"

namespace plato::filter::extension
{

namespace
{

constexpr bool kZeroOut = true;
}  // namespace

LinearMaskFactory::LinearMaskFactory(const NodalVector& aNodalCoordinates,
                                     const CenterVector& aCenters,
                                     const SearchRadius aSearchRadius,
                                     const int aMaximumConnectivityEstimate,
                                     const boost::mpi::communicator& aCommunicator)
    : mCommunicator(aCommunicator),
      mSearchRadius(aSearchRadius.mValue),
      mMaximumConnectivityEstimate(aMaximumConnectivityEstimate),
      mRowCenterCoordinates(aCenters.mValue),
      mNodalCoordinates(createNodalCoordinates(aNodalCoordinates.mValue)),
      mLocalSearchPointWithIdentifiers(detail::stk_search_points(mNodalCoordinates, mCommunicator.rank()))
{
    generateDistanceMap(
        static_cast<third_party_integration::tpetra::TpetraGlobalOrdinal>(aNodalCoordinates.mValue.size()));
}

third_party_integration::tpetra::TpetraMultiVector LinearMaskFactory::createNodalCoordinates(
    const std::vector<third_party_integration::common::Coordinate>& aNodalCoordinates)
{
    const auto tCommunicator(Teuchos::rcp(new Teuchos::MpiComm<int>(mCommunicator)));
    auto tNodalCoordinates = third_party_integration::tpetra::TpetraMultiVector(
        Teuchos::rcp(new third_party_integration::tpetra::TpetraMap(
            aNodalCoordinates.size(), third_party_integration::tpetra::kIndexBase, tCommunicator)),
        third_party_integration::tpetra::kNumberOfCartesianDimensions, kZeroOut);
    third_party_integration::tpetra::distribute_on_tpetra_multivector(aNodalCoordinates, tNodalCoordinates);
    return tNodalCoordinates;
}

auto LinearMaskFactory::returnMask() const -> const third_party_integration::tpetra::TpetraCRSMatrix&
{
    return *mLinearMask;
}

auto LinearMaskFactory::generateRow(third_party_integration::common::Coordinate aCenter)
    -> std::pair<TpetraGlobalOrdinalVector, TpetraScalarVector>
{
    const auto tSearchResults = third_party_integration::stk_search::find_points_in_sphere(
        aCenter, mSearchRadius, mLocalSearchPointWithIdentifiers, mCommunicator);

    constexpr bool tZeroOut = true;
    auto tRow = third_party_integration::tpetra::TpetraVector(mNodalCoordinates.getMap(), tZeroOut);

    double tSum = 0;
    for (const auto [tLocalIndex] : utilities::MultidimensionalRange{
             third_party_integration::tpetra::number_of_local_elements(mNodalCoordinates.getMap())})
    {
        const third_party_integration::stk_search::Identifier tLocalIdentifier{tLocalIndex, mCommunicator.rank()};
        if (third_party_integration::stk_search::is_in_search_results(tLocalIdentifier, tSearchResults))
        {
            const third_party_integration::common::Coordinate tLocalCoordinate =
                third_party_integration::tpetra::multivector_coordinate(mNodalCoordinates, tLocalIndex);

            const double tDistance = third_party_integration::common::magnitude(aCenter - tLocalCoordinate);
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

void LinearMaskFactory::generateDistanceMap(const third_party_integration::tpetra::TpetraGlobalOrdinal aNumberOfRows)
{
    const auto tCommunicator(Teuchos::rcp(new Teuchos::MpiComm<int>(mCommunicator)));
    auto tCrsRowMap = Teuchos::rcp(new third_party_integration::tpetra::TpetraMap(
        mRowCenterCoordinates.size(), third_party_integration::tpetra::kIndexBase, tCommunicator));
    auto tCrsDomainMap = Teuchos::rcp(new third_party_integration::tpetra::TpetraMap(
        aNumberOfRows, third_party_integration::tpetra::kIndexBase, tCommunicator));

    mLinearMask = Teuchos::RCP<third_party_integration::tpetra::TpetraCRSMatrix>{
        new third_party_integration::tpetra::TpetraCRSMatrix(tCrsRowMap, mMaximumConnectivityEstimate)};

    for (const auto [tGlobalIndex] : utilities::MultidimensionalRange{
             static_cast<third_party_integration::tpetra::TpetraGlobalOrdinal>(mRowCenterCoordinates.size())})
    {
        const third_party_integration::common::Coordinate tGlobalCoordinate = mRowCenterCoordinates[tGlobalIndex];
        auto [tGlobalNonZeroIndices, tGlobalNonZeroWeights] = generateRow(tGlobalCoordinate);

        mLinearMask->insertGlobalValues(
            tGlobalIndex,
            Teuchos::ArrayView<third_party_integration::tpetra::TpetraGlobalOrdinal>(tGlobalNonZeroIndices),
            Teuchos::ArrayView<third_party_integration::tpetra::TpetraScalar>(tGlobalNonZeroWeights));
    }
    mLinearMask->fillComplete(tCrsDomainMap, tCrsRowMap);
}

namespace detail
{

double linear_ramp_weight(const Distance aDistance, const SearchRadius aSearchRadius)
{
    return std::max(0.0, 1.0 - aDistance.mValue / aSearchRadius.mValue);
}

auto normalize_nonzero_weights(const third_party_integration::tpetra::TpetraVector& aRowVector,
                               const RowSum aRowSum,
                               const EstimatedConnectivity aEstimatedConnectivity)
    -> std::pair<TpetraGlobalOrdinalVector, TpetraScalarVector>
{
    std::vector<third_party_integration::tpetra::TpetraScalar> tNonZeroWeightVector;
    std::vector<third_party_integration::tpetra::TpetraGlobalOrdinal> tNonZeroGlobalIndices;
    tNonZeroWeightVector.reserve(aEstimatedConnectivity.mValue);
    tNonZeroGlobalIndices.reserve(aEstimatedConnectivity.mValue);

    for (const auto [tLocalIndex] : utilities::MultidimensionalRange{
             third_party_integration::tpetra::number_of_local_elements(aRowVector.getMap())})
    {
        const third_party_integration::tpetra::TpetraScalar tReplacementWeight =
            aRowVector.getData()[tLocalIndex] / aRowSum.mValue;
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
    std::vector<third_party_integration::stk_search::SearchPointWithIdentifier> tLocalSearchPointWithIdentifiers(
        third_party_integration::tpetra::number_of_local_elements(aNodalCoordinates.getMap()));

    for (const auto [tLocalIndex] : utilities::MultidimensionalRange{
             third_party_integration::tpetra::number_of_local_elements(aNodalCoordinates.getMap())})
    {
        const third_party_integration::stk_search::Identifier tIdentifier{tLocalIndex, aRank};
        const third_party_integration::common::Coordinate tCoordinate =
            third_party_integration::tpetra::multivector_coordinate(aNodalCoordinates, tLocalIndex);
        tLocalSearchPointWithIdentifiers[tLocalIndex] = third_party_integration::stk_search::SearchPointWithIdentifier(
            {third_party_integration::stk_search::convert_coordinate(tCoordinate), tIdentifier});
    }
    return tLocalSearchPointWithIdentifiers;
}

}  // namespace detail

}  // namespace plato::filter::extension
