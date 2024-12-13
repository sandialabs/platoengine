#ifndef PLATO_ANALYSIS_UTILITIES
#define PLATO_ANALYSIS_UTILITIES

namespace plato::analysis
{
struct AnalysisDomainMesh;
}

namespace plato::analysis
{
/// @brief Sets all scalar field values in @a aAnalysisDomainMesh to zero, and retains the original indices.
auto zero_scalar_field(analysis::AnalysisDomainMesh &&aAnalysisDomainMesh) -> analysis::AnalysisDomainMesh;
}  // namespace plato::analysis

#endif
