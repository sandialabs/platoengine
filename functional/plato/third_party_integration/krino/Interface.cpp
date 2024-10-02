#include "plato/third_party_integration/krino/Interface.hpp"

#include "plato/third_party_integration/krino/KrinoWrapper.hpp"
#include "plato/third_party_integration/krino/LevelsetPrimitives.hpp"

namespace plato::third_party_integration::krino
{
std::unordered_map<stk::mesh::EntityId, InterfaceNodeDXDP> generate_computational_mesh(
    const std::string &aBackgroundMeshName,
    const std::string &aCutMesh,
    const std::vector<double> &aLevelsetValues,
    const bool aIncludeVoidRegion)
{
    KrinoWrapper tKrinoWrapper(aBackgroundMeshName, aIncludeVoidRegion);
    tKrinoWrapper.setLevelsetValues(aLevelsetValues);
    tKrinoWrapper.cutMesh();
    tKrinoWrapper.writeMesh(aCutMesh);
    return tKrinoWrapper.getSensitivities();
}

std::vector<double> initialize_mesh_with_levelset_primitives(const std::string &aBackgroundMeshName,
                                                             const std::string &aCutMesh,
                                                             const LevelsetPrimitives &aLevelsetPrimitives,
                                                             const bool aIncludeVoidRegion)
{
    KrinoWrapper tKrinoWrapper(aBackgroundMeshName, aIncludeVoidRegion);
    tKrinoWrapper.initializeLevelsetsFromPrimitives(aLevelsetPrimitives);
    tKrinoWrapper.cutMesh();
    tKrinoWrapper.writeMesh(aCutMesh);
    return tKrinoWrapper.getLevelsetValues();
}

}  // namespace plato::third_party_integration::krino
