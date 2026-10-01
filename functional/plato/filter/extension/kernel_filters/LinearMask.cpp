#include "plato/filter/extension/kernel_filters/LinearMask.hpp"

#include <Teuchos_BLAS_types.hpp>
#include <Teuchos_DefaultMpiComm.hpp>
#include <Teuchos_RCP.hpp>
#include <Teuchos_RCPDecl.hpp>
#include <boost/mpi/collectives.hpp>
#include <boost/mpi/communicator.hpp>
#include <memory>
#include <utility>
#include <vector>

#include "plato/third_party_integration/tpetra/Utilities.hpp"

namespace plato::filter::extension::kernel_filters
{
namespace
{
constexpr auto kInitializeToZero = true;
[[nodiscard]] auto to_teuchos_communicator(const boost::mpi::communicator& aCommunicator)
    -> Teuchos::RCP<Teuchos::MpiComm<int>>
{
    return Teuchos::rcp(std::make_unique<Teuchos::MpiComm<int>>(aCommunicator).release());
}
}  // namespace

LinearMask::LinearMask(third_party_integration::tpetra::TpetraCRSMatrix aLinearMask,
                       boost::mpi::communicator aCommunicator)
    : mCommunicator{std::move(aCommunicator)}, mLinearMask{std::move(aLinearMask)}
{
}

namespace
{
[[nodiscard]] auto matrix_multiplication(const std::vector<double>& aValues,
                                         const boost::mpi::communicator& aCommunicator,
                                         const third_party_integration::tpetra::TpetraCRSMatrix& aLinearMask,
                                         third_party_integration::tpetra::TpetraVector& aInputVector,
                                         third_party_integration::tpetra::TpetraVector& aOutputVector,
                                         const Teuchos::ETransp aTeuchosTransposeOption) -> std::vector<double>
{
    const auto tTeuchosCommunicator = to_teuchos_communicator(aCommunicator);
    third_party_integration::tpetra::distribute_on_tpetra_vector(aValues, aInputVector);
    aLinearMask.apply(aInputVector, aOutputVector, aTeuchosTransposeOption);
    return third_party_integration::tpetra::reduce_tpetra_vector(aOutputVector, tTeuchosCommunicator);
}
}  // namespace

auto LinearMask::matrixMultiply(const std::vector<double>& aValues) const -> std::vector<double>
{
    auto tRowVector = third_party_integration::tpetra::TpetraVector{mLinearMask.getRangeMap(), kInitializeToZero};
    auto tColumnVector = third_party_integration::tpetra::TpetraVector{mLinearMask.getDomainMap(), kInitializeToZero};
    return matrix_multiplication(aValues, mCommunicator, mLinearMask, tColumnVector, tRowVector, Teuchos::NO_TRANS);
}

auto LinearMask::transposeMatrixMultiply(const std::vector<double>& aValues) const -> std::vector<double>
{
    auto tRowVector = third_party_integration::tpetra::TpetraVector{mLinearMask.getRangeMap(), kInitializeToZero};
    auto tColumnVector = third_party_integration::tpetra::TpetraVector{mLinearMask.getDomainMap(), kInitializeToZero};
    return matrix_multiplication(aValues, mCommunicator, mLinearMask, tRowVector, tColumnVector, Teuchos::TRANS);
}

auto LinearMask::size() const -> std::pair<third_party_integration::tpetra::TpetraGlobalOrdinal,
                                           third_party_integration::tpetra::TpetraGlobalOrdinal>
{
    return {mLinearMask.getGlobalNumRows(), mLinearMask.getGlobalNumCols()};
}

}  // namespace plato::filter::extension::kernel_filters
