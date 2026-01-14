#include "plato/filter/extension/kernel_filters/LinearMask.hpp"

#include <Teuchos_ArrayViewDecl.hpp>
#include <Teuchos_EReductionType.hpp>
#include <boost/mpi/collectives.hpp>
#include <set>

#include "plato/filter/extension/kernel_filters/SphericalMaskUtilities.hpp"

namespace plato::filter::extension::kernel_filters
{
LinearMask::LinearMask(third_party_integration::tpetra::TpetraCRSMatrix aLinearMask,
                       const boost::mpi::communicator& aCommunicator)
    : mCommunicator{aCommunicator}, mLinearMask{std::move(aLinearMask)}
{
}

std::vector<double> LinearMask::matrixMultiply(const std::vector<double>& aValues) const
{
    auto [tRowVector, tColumnVector] =
        third_party_integration::tpetra::create_zeroed_row_and_column_vectors_from_crs_map(
            mLinearMask, Teuchos::rcp(new Teuchos::MpiComm<int>(mCommunicator)));

    third_party_integration::tpetra::distribute_on_tpetra_vector(aValues, tColumnVector);
    mLinearMask.apply(tColumnVector, tRowVector);

    return third_party_integration::tpetra::reduce_tpetra_vector(
        tRowVector, Teuchos::rcp(new Teuchos::MpiComm<int>(mCommunicator)));
}

std::vector<double> LinearMask::transposeMatrixMultiply(const std::vector<double>& aValues) const
{
    auto [tRowVector, tColumnVector] =
        third_party_integration::tpetra::create_zeroed_row_and_column_vectors_from_crs_map(
            mLinearMask, Teuchos::rcp(new Teuchos::MpiComm<int>(mCommunicator)));

    third_party_integration::tpetra::distribute_on_tpetra_vector(aValues, tRowVector);
    mLinearMask.apply(tRowVector, tColumnVector, Teuchos::TRANS);

    return third_party_integration::tpetra::reduce_tpetra_vector(
        tColumnVector, Teuchos::rcp(new Teuchos::MpiComm<int>(mCommunicator)));
}

std::pair<third_party_integration::tpetra::TpetraGlobalOrdinal, third_party_integration::tpetra::TpetraGlobalOrdinal>
LinearMask::size() const
{
    return {mLinearMask.getGlobalNumRows(), mLinearMask.getGlobalNumCols()};
}

}  // namespace plato::filter::extension::kernel_filters
