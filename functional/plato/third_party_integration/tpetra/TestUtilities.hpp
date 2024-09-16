#ifndef PLATO_THIRDPARTYINTEGRATION_TPETRA_TESTUTILITIES
#define PLATO_THIRDPARTYINTEGRATION_TPETRA_TESTUTILITIES

#include "plato/third_party_integration/tpetra/Utilities.hpp"
#include "plato/utilities/NamedType.hpp"

namespace plato::third_party_integration::tpetra
{
using VectorSize = utilities::NamedType<size_t, struct VectorSizeTag>;

/// @brief Helper function create a tpetra map of size @a aSize using comm @a aCommunicator
[[nodiscard]] Teuchos::RCP<const TpetraMap> create_contiguous_map(
    const Tpetra::global_size_t aSize, const Teuchos::RCP<const Teuchos::Comm<int>>& aCommunicator);

/// @brief Helper function that creates a vector of zeros sized @a aSize except for 1 at entry @a aGlobalIndex
[[nodiscard]] TpetraVector create_projection_vector(const VectorSize aSize, const TpetraGlobalOrdinal aGlobalIndex);

/// @brief Helper function that does to projection operations to retrieve a global entry from a CRS matrix @a aMatrix at
/// index @a aGlobalIndexI and @a aGlobalIndexJ. This is for testing and should not get around...
[[nodiscard]] TpetraScalar get_entry(const TpetraCRSMatrix& aMatrix,
                                     const TpetraGlobalOrdinal aGlobalIndexI,
                                     const TpetraGlobalOrdinal aGlobalIndexJ);

}  // namespace plato::third_party_integration::tpetra
#endif
