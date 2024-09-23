#include "plato/third_party_integration/tpetra/TestUtilities.hpp"

namespace plato::third_party_integration::tpetra
{
Teuchos::RCP<const TpetraMap> create_contiguous_map(const Tpetra::global_size_t aSize,
                                                    const Teuchos::RCP<const Teuchos::Comm<int>>& aCommunicator)
{
    return Teuchos::rcp(new TpetraMap(aSize, kIndexBase, aCommunicator));
}

TpetraVector create_projection_vector(const VectorSize aSize, const TpetraGlobalOrdinal aGlobalIndex)
{
    const auto tCommunicator = Tpetra::getDefaultComm();
    const auto tContiguousMap = create_contiguous_map(aSize.mValue, tCommunicator);
    constexpr bool tZeroOut = true;
    auto tVector = TpetraVector(tContiguousMap, tZeroOut);
    tVector.replaceGlobalValue(aGlobalIndex, 1);
    return tVector;
}

TpetraScalar get_entry(const TpetraCRSMatrix& aMatrix,
                       const TpetraGlobalOrdinal aGlobalIndexI,
                       const TpetraGlobalOrdinal aGlobalIndexJ)
{
    const auto tNRows = aMatrix.getGlobalNumRows();
    const auto tMColumns = aMatrix.getGlobalNumCols();
    const auto tCommunicator = Tpetra::getDefaultComm();
    const auto tContiguousMap = create_contiguous_map(tNRows, tCommunicator);
    constexpr bool tZeroOut = true;
    auto tResult = TpetraVector(tContiguousMap, tZeroOut);

    const auto tProjectionI = create_projection_vector(VectorSize{tNRows}, aGlobalIndexI);
    const auto tProjectionJ = create_projection_vector(VectorSize{tMColumns}, aGlobalIndexJ);
    aMatrix.apply(tProjectionJ, tResult);
    return tProjectionI.dot(tResult);
}
}  // namespace plato::third_party_integration::tpetra
