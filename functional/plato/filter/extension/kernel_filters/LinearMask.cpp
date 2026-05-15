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

auto LinearMask::matrixMultiply(const std::vector<double>& aValues) const -> std::vector<double>
{
    const auto tTeuchosCommunicator = to_teuchos_communicator(mCommunicator);
    auto [tRowVector, tColumnVector] =
        third_party_integration::tpetra::create_zeroed_row_and_column_vectors_from_crs_map(mLinearMask,
                                                                                           tTeuchosCommunicator);

    third_party_integration::tpetra::distribute_on_tpetra_vector(aValues, tColumnVector);
    mLinearMask.apply(tColumnVector, tRowVector);

    return third_party_integration::tpetra::reduce_tpetra_vector(tRowVector, tTeuchosCommunicator);
}

auto LinearMask::transposeMatrixMultiply(const std::vector<double>& aValues) const -> std::vector<double>
{
    const auto tTeuchosCommunicator = to_teuchos_communicator(mCommunicator);
    auto [tRowVector, tColumnVector] =
        third_party_integration::tpetra::create_zeroed_row_and_column_vectors_from_crs_map(mLinearMask,
                                                                                           tTeuchosCommunicator);

    third_party_integration::tpetra::distribute_on_tpetra_vector(aValues, tRowVector);
    mLinearMask.apply(tRowVector, tColumnVector, Teuchos::TRANS);

    return third_party_integration::tpetra::reduce_tpetra_vector(tColumnVector, tTeuchosCommunicator);
}

auto LinearMask::size() const -> std::pair<third_party_integration::tpetra::TpetraGlobalOrdinal,
                                           third_party_integration::tpetra::TpetraGlobalOrdinal>
{
    return {mLinearMask.getGlobalNumRows(), mLinearMask.getGlobalNumCols()};
}

}  // namespace plato::filter::extension::kernel_filters
