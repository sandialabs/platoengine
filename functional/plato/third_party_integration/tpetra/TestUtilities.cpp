#include "plato/third_party_integration/tpetra/TestUtilities.hpp"

#include <Teuchos_Comm.hpp>
#include <Teuchos_RCP.hpp>
#include <Teuchos_RCPDecl.hpp>
#include <Tpetra_ConfigDefs.hpp>
#include <Tpetra_Core.hpp>
#include <memory>

#include "plato/third_party_integration/tpetra/Utilities.hpp"

namespace plato::third_party_integration::tpetra
{
auto create_contiguous_map(const Tpetra::global_size_t aSize,
                           const Teuchos::RCP<const Teuchos::Comm<int>>& aCommunicator) -> Teuchos::RCP<const TpetraMap>
{
    return Teuchos::rcp(std::make_unique<TpetraMap>(aSize, kIndexBase, aCommunicator).release());
}

auto create_projection_vector(const VectorSize aSize, const TpetraGlobalOrdinal aGlobalIndex) -> TpetraVector
{
    const auto tCommunicator = Tpetra::getDefaultComm();
    const auto tContiguousMap = create_contiguous_map(aSize.mValue, tCommunicator);
    constexpr bool tZeroOut = true;
    auto tVector = TpetraVector(tContiguousMap, tZeroOut);
    tVector.replaceGlobalValue(aGlobalIndex, 1);
    return tVector;
}

auto get_entry(const TpetraCRSMatrix& aMatrix,
               const TpetraGlobalOrdinal aGlobalIndexI,
               const TpetraGlobalOrdinal aGlobalIndexJ) -> TpetraScalar
{
    const auto tNRows = aMatrix.getGlobalNumRows();
    const auto tColumns = aMatrix.getGlobalNumCols();
    const auto tCommunicator = Tpetra::getDefaultComm();
    const auto tContiguousMap = create_contiguous_map(tNRows, tCommunicator);
    constexpr bool tZeroOut = true;
    auto tResult = TpetraVector(tContiguousMap, tZeroOut);

    const auto tProjectionI = create_projection_vector(VectorSize{tNRows}, aGlobalIndexI);
    const auto tProjectionJ = create_projection_vector(VectorSize{tColumns}, aGlobalIndexJ);
    aMatrix.apply(tProjectionJ, tResult);
    return tProjectionI.dot(tResult);
}
}  // namespace plato::third_party_integration::tpetra
