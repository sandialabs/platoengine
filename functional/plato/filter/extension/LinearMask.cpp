#include "plato/filter/extension/LinearMask.hpp"

#include <Teuchos_ArrayViewDecl.hpp>
#include <Teuchos_EReductionType.hpp>
#include <boost/mpi/collectives.hpp>
#include <set>

#include "plato/filter/extension/LinearMaskFactory.hpp"

namespace plato::filter::extension
{

LinearMask::LinearMask(const NodalVector& aNodalCoordinates,
                       const CenterVector& aCentroids,
                       const SearchRadius aSearchRadius,
                       const int aMaximumConnectivityEstimate,
                       const boost::mpi::communicator& aCommunicator)
    : mCommunicator(aCommunicator),
      mLinearMask(
          LinearMaskFactory{aNodalCoordinates, aCentroids, aSearchRadius, aMaximumConnectivityEstimate, aCommunicator}
              .returnMask())
{
}

LinearMask::LinearMask(const NodalVector& aNodalCoordinates,
                       const SearchRadius aSearchRadius,
                       const int aMaximumConnectivityEstimate,
                       const boost::mpi::communicator& aCommunicator)
    : mCommunicator(aCommunicator),
      mLinearMask(LinearMaskFactory{aNodalCoordinates, CenterVector{aNodalCoordinates.mValue}, aSearchRadius,
                                    aMaximumConnectivityEstimate, aCommunicator}
                      .returnMask())
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

}  // namespace plato::filter::extension
