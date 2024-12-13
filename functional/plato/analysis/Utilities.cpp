#include "plato/analysis/Utilities.hpp"

#include "plato/analysis/AnalysisDomainMesh.hpp"
#include "plato/analysis/AnalysisDomainMeshSequentialView.hpp"

namespace plato::analysis
{
auto zero_scalar_field(analysis::AnalysisDomainMesh &&aAnalysisDomainMesh) -> analysis::AnalysisDomainMesh
{
    for (auto tScalarFieldValue : analysis::AnalysisDomainMeshMutableSequentialView{aAnalysisDomainMesh})
    {
        const auto &tFieldIndices = static_cast<analysis::ScalarFieldValue>(tScalarFieldValue);
        tScalarFieldValue = analysis::ScalarFieldValue{tFieldIndices.mGlobalMeshEntityID,
                                                       tFieldIndices.mDesignVariableVectorIndex, 0.0};
    }
    return aAnalysisDomainMesh;
}
}  // namespace plato::analysis
