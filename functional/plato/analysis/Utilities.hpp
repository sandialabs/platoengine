#ifndef PLATO_ANALYSIS_UTILITIES
#define PLATO_ANALYSIS_UTILITIES

#include <vector>

#include "plato/analysis/AnalysisDomainMesh.hpp"

namespace plato::analysis
{
/// @brief Sets all scalar field values in @a aAnalysisDomainMesh to zero, and retains the original indices.
[[nodiscard]] auto zero_scalar_field(AnalysisDomainMesh&& aAnalysisDomainMesh) -> AnalysisDomainMesh;

/// @brief Removes all blocks in @a aAnalysisDomainMesh with IDs given by @a aBlockIDs.
///
/// If an ID in @a aBlockIDs does not have an associated entry in @a aAnalysisDomainMesh, then it is ignored.
[[nodiscard]] auto remove_block_fields(AnalysisDomainMesh&& aAnalysisDomainMesh,
                                       const std::vector<AnalysisDomainMesh::BlockIDType>& aBlockIDs)
    -> AnalysisDomainMesh;

/// @brief Computes a hash of @a aScalarFieldValue.
[[nodiscard]] auto hash_value(const ScalarFieldValue& aScalarFieldValue) -> std::size_t;

/// @brief Computes a hash of the scalar field values and mesh path name stored in @a aAnlysisDomainMesh.
/// @warning This does not change if the nodal coordinates in the underlying mesh change, however this does hash the
/// mesh file path.
[[nodiscard]] auto hash_value(const AnalysisDomainMesh& aAnalysisDomainMesh) -> std::size_t;
}  // namespace plato::analysis

#endif
