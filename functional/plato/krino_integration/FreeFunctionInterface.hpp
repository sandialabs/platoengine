#ifndef PLATO_KRINO_INTEGRATION_FREE_FUNCTION_INTERFACE
#define PLATO_KRINO_INTEGRATION_FREE_FUNCTION_INTERFACE

/*
#include <Akri_BoundingBoxMesh.hpp>
#include <Akri_LevelSet.hpp>
#include <Akri_LevelSetPolicy.hpp>
#include <Akri_MeshFromFile.hpp>
#include <Akri_Phase_Support.hpp>
#include <memory>
#include <stk_mesh/base/MetaData.hpp>

*/
// #include "stk_mesh/base/Types.hpp"
#include "Utilities.hpp"

namespace plato::krino_integration
{
std::map<stk::mesh::EntityId, InterfaceNode_DXDP> generateComputationalMesh(const std::string &aBackgroundMeshName,
                                                                            const std::string &aCutMesh,
                                                                            const std::vector<double> &aLevelsetValues,
                                                                            const bool aIncludeVoidRegion);

std::vector<double> initializeMeshWithLevelsetPrimitives(const std::string &aBackgroundMeshName,
                                                         const std::string &aCutMesh,
                                                         const LevelsetPrimitives &aLevelsetPrimitives,
                                                         const bool aIncludeVoidRegion);

}  // namespace plato::krino_integration

#endif  // PLATO_KRINO_INTEGRATION_FREE_FUNCTION_INTERFACE
