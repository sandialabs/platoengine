#ifndef PLATO_ANALYSIS_UNITTEST_UTILITIES
#define PLATO_ANALYSIS_UNITTEST_UTILITIES

#include "plato/analysis/AnalysisDomainMesh.hpp"

namespace plato::analysis
{
[[nodiscard]] inline bool operator==(const ScalarFieldValue& aLHS, const ScalarFieldValue& aRHS)
{
    return aLHS.mValue == aRHS.mValue && aLHS.mGlobalMeshEntityID == aRHS.mGlobalMeshEntityID &&
           aLHS.mDesignVariableVectorIndex == aRHS.mDesignVariableVectorIndex;
}
}  // namespace plato::analysis

#endif
