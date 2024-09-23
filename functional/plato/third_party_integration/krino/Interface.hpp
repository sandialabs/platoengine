#ifndef PLATO_THIRD_PARTY_INTEGRATION_KRINO_INTERFACE
#define PLATO_THIRD_PARTY_INTEGRATION_KRINO_INTERFACE

#include "Utilities.hpp"

namespace plato::third_party_integration::krino
{
std::map<stk::mesh::EntityId, InterfaceNode_DXDP> generateComputationalMesh(const std::string &aBackgroundMeshName,
                                                                            const std::string &aCutMesh,
                                                                            const std::vector<double> &aLevelsetValues,
                                                                            const bool aIncludeVoidRegion);

std::vector<double> initializeMeshWithLevelsetPrimitives(const std::string &aBackgroundMeshName,
                                                         const std::string &aCutMesh,
                                                         const LevelsetPrimitives &aLevelsetPrimitives,
                                                         const bool aIncludeVoidRegion);

}  // namespace plato::third_party_integration::krino

#endif  // PLATO_THIRD_PARTY_INTEGRATION_KRINO_INTERFACE
