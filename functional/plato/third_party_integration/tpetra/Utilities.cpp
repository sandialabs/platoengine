#include "plato/third_party_integration/tpetra/Utilities.hpp"

namespace plato::third_party_integration::tpetra
{

TpetraLocalOrdinal number_of_local_elements(const Teuchos::RCP<const TpetraMap>& aTpetraMap)
{
    return static_cast<TpetraLocalOrdinal>(aTpetraMap->getLocalNumElements());
}

std::pair<TpetraVector, TpetraVector> create_zeroed_row_and_column_vectors_from_crs_map(
    const TpetraCRSMatrix& aCRSMatrix, const Teuchos::RCP<const Teuchos::Comm<int>>& aCommunicator)
{
    const auto tRowContiguousMap =
        Teuchos::rcp(new TpetraMap(aCRSMatrix.getGlobalNumRows(), kIndexBase, aCommunicator));
    const auto tColumnContiguousMap =
        Teuchos::rcp(new TpetraMap(aCRSMatrix.getGlobalNumCols(), kIndexBase, aCommunicator));
    constexpr bool tZeroOut = true;
    return {TpetraVector(tRowContiguousMap, tZeroOut), TpetraVector(tColumnContiguousMap, tZeroOut)};
}

void distribute_on_tpetra_vector(const std::vector<double>& aSerialVector, TpetraVector& aVector)
{
    assert(aSerialVector.size() == aVector.getMap()->getGlobalNumElements());
    for (TpetraLocalOrdinal tIndex = 0; tIndex < number_of_local_elements(aVector.getMap()); ++tIndex)
    {
        const auto tGlobalID = aVector.getMap()->getGlobalElement(tIndex);
        aVector.replaceLocalValue(tIndex, aSerialVector[tGlobalID]);
    }
}

std::vector<double> reduce_tpetra_vector(const TpetraVector& aVector,
                                         const Teuchos::RCP<const Teuchos::Comm<int>>& aCommunicator)
{
    std::vector<double> tResult(aVector.getMap()->getGlobalNumElements(), 0);
    for (TpetraGlobalOrdinal tGlobalIndex = 0;
         tGlobalIndex < static_cast<TpetraGlobalOrdinal>(aVector.getMap()->getGlobalNumElements()); ++tGlobalIndex)
    {
        const auto tLocalIndex = aVector.getMap()->getLocalElement(tGlobalIndex);
        double tLocalValue = tLocalIndex >= 0 ? aVector.getData()[tLocalIndex] : 0.0;
        Teuchos::reduceAll<TeuchosCommOrdinal, double>(*aCommunicator, Teuchos::REDUCE_SUM, tLocalValue,
                                                       Teuchos::outArg(tResult[tGlobalIndex]));
    }

    return tResult;
}

common::Coordinate multivector_coordinate(const TpetraMultiVector& aMultiVector, const TpetraLocalOrdinal aRow)
{
    assert(aMultiVector.getNumVectors() == kNumberOfCartesianDimensions);
    return {aMultiVector.getVector(0)->getData()[aRow], aMultiVector.getVector(1)->getData()[aRow],
            aMultiVector.getVector(2)->getData()[aRow]};
}

void distribute_on_tpetra_multivector(const std::vector<common::Coordinate>& aSerialVector,
                                      TpetraMultiVector& aMultiVector)
{
    assert(aSerialVector.size() == aMultiVector.getMap()->getGlobalNumElements());
    for (TpetraLocalOrdinal tIndex = 0; tIndex < number_of_local_elements(aMultiVector.getMap()); ++tIndex)
    {
        const auto tGlobalID = aMultiVector.getMap()->getGlobalElement(tIndex);
        aMultiVector.replaceLocalValue(tIndex, 0, aSerialVector[tGlobalID].x);
        aMultiVector.replaceLocalValue(tIndex, 1, aSerialVector[tGlobalID].y);
        aMultiVector.replaceLocalValue(tIndex, 2, aSerialVector[tGlobalID].z);
    }
}

}  // namespace plato::third_party_integration::tpetra
