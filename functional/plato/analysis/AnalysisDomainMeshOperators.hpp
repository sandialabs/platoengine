#ifndef PLATO_ANALYSIS_ANALYSISDOMAINMESHOPERATORS
#define PLATO_ANALYSIS_MESHDESIGNVARIABLES

#include "plato/analysis/AnalysisDomainMesh.hpp"

namespace plato::analysis
{
[[nodiscard]] inline bool operator==(const ScalarFieldValue& aLHS, const ScalarFieldValue& aRHS)
{
    return aLHS.mValue == aRHS.mValue && aLHS.mGlobalMeshEntityID == aRHS.mGlobalMeshEntityID &&
           aLHS.mDesignVariableVectorIndex == aRHS.mDesignVariableVectorIndex;
}

[[nodiscard]] inline bool operator!=(const ScalarFieldValue& aLHS, const ScalarFieldValue& aRHS)
{
    return !(aLHS == aRHS);
}
}  // namespace plato::analysis

#endif
