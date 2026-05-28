#ifndef PLATO_ANALYSIS_ANALYSISDOMAINMESHOPERATORS
#define PLATO_ANALYSIS_ANALYSISDOMAINMESHOPERATORS

#include "plato/analysis/AnalysisDomainMesh.hpp"

namespace plato::analysis
{
[[nodiscard]] inline auto operator==(const ScalarFieldValue& aLHS, const ScalarFieldValue& aRHS) -> bool
{
    return aLHS.mValue == aRHS.mValue && aLHS.mGlobalMeshEntityID == aRHS.mGlobalMeshEntityID &&
           aLHS.mDesignVariableVectorIndex == aRHS.mDesignVariableVectorIndex;
}

[[nodiscard]] inline auto operator!=(const ScalarFieldValue& aLHS, const ScalarFieldValue& aRHS) -> bool
{
    return !(aLHS == aRHS);
}
}  // namespace plato::analysis

#endif
