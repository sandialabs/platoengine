#ifndef PLATO_FILTER_EXTENSION_LINEARMASK
#define PLATO_FILTER_EXTENSION_LINEARMASK

#include <boost/mpi/communicator.hpp>
#include <optional>

#include "plato/filter/extension/LinearMaskBuilder.hpp"
#include "plato/third_party_integration/tpetra/Utilities.hpp"

namespace plato::filter::extension
{

/// @brief A linear operator used to implement filtering.
///
/// This class is mainly a wrapper for a sparse matrix and provides members for multiplication and
/// transpose multiplication.
/// Construction is facilitated with LinearMaskFactory.
/// @sa LinearMaskFactory
class LinearMask
{
   public:
    /// @brief Constructs a LinearMask from a TpetraCRSMatrix.
    LinearMask(third_party_integration::tpetra::TpetraCRSMatrix aLinearMask,
               const boost::mpi::communicator& mCommunicator);

    /// @brief Return the size of the distance mask.
    [[nodiscard]] auto size() const -> std::pair<third_party_integration::tpetra::TpetraGlobalOrdinal,
                                                 third_party_integration::tpetra::TpetraGlobalOrdinal>;

    [[nodiscard]] std::vector<double> matrixMultiply(const std::vector<double>& aValues) const;
    [[nodiscard]] std::vector<double> transposeMatrixMultiply(const std::vector<double>& aValues) const;

   private:
    boost::mpi::communicator mCommunicator;
    third_party_integration::tpetra::TpetraCRSMatrix mLinearMask;
};

}  // namespace plato::filter::extension

#endif
