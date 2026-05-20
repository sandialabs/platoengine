#ifndef PLATO_FILTER_EXTENSION_KERNELFILTERS_LINEARMASK
#define PLATO_FILTER_EXTENSION_KERNELFILTERS_LINEARMASK

#include <boost/mpi/communicator.hpp>

#include "plato/filter/extension/kernel_filters/SphericalMaskUtilities.hpp"
#include "plato/third_party_integration/tpetra/Utilities.hpp"

namespace plato::filter::extension::kernel_filters
{

/// @brief A linear operator used to implement filtering.
///
/// This class is mainly a wrapper for a sparse matrix and provides members for multiplication and
/// transpose multiplication.
/// Construction is facilitated with LinearMaskBuilder.
/// @sa LinearMaskBuilder
class LinearMask
{
   public:
    /// @brief Constructs a LinearMask from a TpetraCRSMatrix, which may be constructed from LinearMaskBuilder.
    LinearMask(third_party_integration::tpetra::TpetraCRSMatrix aLinearMask, boost::mpi::communicator aCommunicator);

    /// @brief Return the size of the distance mask. The first entry is the number of rows and the second is the number
    /// of columns.
    [[nodiscard]] auto size() const -> std::pair<third_party_integration::tpetra::TpetraGlobalOrdinal,
                                                 third_party_integration::tpetra::TpetraGlobalOrdinal>;

    [[nodiscard]] auto matrixMultiply(const std::vector<double>& aValues) const -> std::vector<double>;
    [[nodiscard]] auto transposeMatrixMultiply(const std::vector<double>& aValues) const -> std::vector<double>;

   private:
    boost::mpi::communicator mCommunicator;
    third_party_integration::tpetra::TpetraCRSMatrix mLinearMask;
};

}  // namespace plato::filter::extension::kernel_filters

#endif
