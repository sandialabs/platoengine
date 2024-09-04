#ifndef PLATO_DESIGN_VARIABLES_UNITTEST_UTILITIES
#define PLATO_DESIGN_VARIABLES_UNITTEST_UTILITIES

#include "plato/design_variables/MeshDesignVariables.hpp"

namespace plato::design_variables
{
[[nodiscard]] inline bool operator==(const ScalarFieldValue& aLHS, const ScalarFieldValue& aRHS)
{
    return aLHS.mValue == aRHS.mValue && aLHS.mGlobalMeshEntityID == aRHS.mGlobalMeshEntityID &&
           aLHS.mDesignVariableVectorIndex == aRHS.mDesignVariableVectorIndex;
}
}  // namespace plato::design_variables

#endif
