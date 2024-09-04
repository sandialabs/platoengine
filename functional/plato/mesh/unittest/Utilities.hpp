#ifndef PLATO_MESH_UNITTEST_UTILITIES
#define PLATO_MESH_UNITTEST_UTILITIES

#include "plato/mesh/MeshDesignVariables.hpp"

namespace plato::mesh
{
[[nodiscard]] inline bool operator==(const ScalarFieldValue& aLHS, const ScalarFieldValue& aRHS)
{
    return aLHS.mValue == aRHS.mValue && aLHS.mGlobalMeshEntityID == aRHS.mGlobalMeshEntityID &&
           aLHS.mDesignVariableVectorIndex == aRHS.mDesignVariableVectorIndex;
}
}  // namespace plato::mesh

#endif
