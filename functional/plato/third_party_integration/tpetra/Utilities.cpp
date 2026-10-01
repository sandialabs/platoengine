#include "plato/third_party_integration/tpetra/Utilities.hpp"

#include <Teuchos_Comm.hpp>
#include <Teuchos_CommHelpers.hpp>
#include <Teuchos_EReductionType.hpp>
#include <Teuchos_RCPDecl.hpp>
#include <cassert>
#include <cstddef>
#include <ranges>
#include <vector>

#include "plato/third_party_integration/common/Vector3.hpp"

namespace plato::third_party_integration::tpetra
{

auto number_of_local_elements(const Teuchos::RCP<const TpetraMap>& aTpetraMap) -> TpetraLocalOrdinal
{
    return static_cast<TpetraLocalOrdinal>(aTpetraMap->getLocalNumElements());
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

auto reduce_tpetra_vector(const TpetraVector& aVector,
                          const Teuchos::RCP<const Teuchos::Comm<int>>& aCommunicator) -> std::vector<double>
{
    const auto tGlobalSize = static_cast<std::size_t>(aVector.getMap()->getGlobalNumElements());
    auto tLocalValues = std::vector<double>(tGlobalSize, 0.0);
    auto tGlobalValues = std::vector<double>(tGlobalSize, 0.0);

    const auto tLocalData = aVector.getData();

    for (const auto tLocalIndex : std::views::iota(TpetraLocalOrdinal{0}, number_of_local_elements(aVector.getMap())))
    {
        const auto tGlobalIndex = static_cast<std::size_t>(aVector.getMap()->getGlobalElement(tLocalIndex));
        tLocalValues[tGlobalIndex] = tLocalData[tLocalIndex];
    }
    Teuchos::reduceAll<TeuchosCommOrdinal, double>(*aCommunicator, Teuchos::REDUCE_SUM,
                                                   static_cast<TeuchosCommOrdinal>(tGlobalValues.size()),
                                                   tLocalValues.data(), tGlobalValues.data());

    return tGlobalValues;
}

auto multivector_coordinate(const TpetraMultiVector& aMultiVector, const TpetraLocalOrdinal aRow) -> common::Coordinate
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
