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
}  // namespace plato::analysis

#endif
