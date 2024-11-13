#include "plato/third_party_integration/krino/Interface.hpp"

#include "plato/third_party_integration/krino/KrinoWrapper.hpp"
#include "plato/third_party_integration/krino/LevelsetPrimitives.hpp"

namespace plato::third_party_integration::krino
{

auto generate_computational_mesh(const BackgroundMeshFilePath &aBackgroundMeshName,
                                 const CutMeshFilePath &aCutMesh,
                                 const std::vector<double> &aLevelsetValues,
                                 const bool aIncludeVoidRegion)
    -> const std::unordered_map<stk::mesh::EntityId, InterfaceNodeDXDP> &
{
    KrinoWrapper tKrinoWrapper(aBackgroundMeshName.mValue, aIncludeVoidRegion);
    tKrinoWrapper.setLevelsetValues(aLevelsetValues);
    tKrinoWrapper.cutMesh();
    tKrinoWrapper.writeMesh(aCutMesh.mValue);
    return tKrinoWrapper.getSensitivities();
}

std::vector<double> initialize_mesh_with_levelset_primitives(const BackgroundMeshFilePath &aBackgroundMeshName,
                                                             const CutMeshFilePath &aCutMesh,
                                                             const LevelsetPrimitives &aLevelsetPrimitives,
                                                             const bool aIncludeVoidRegion)
{
    KrinoWrapper tKrinoWrapper(aBackgroundMeshName.mValue, aIncludeVoidRegion);
    tKrinoWrapper.initializeLevelsetsFromPrimitives(aLevelsetPrimitives);
    tKrinoWrapper.cutMesh();
    tKrinoWrapper.writeMesh(aCutMesh.mValue);
    return tKrinoWrapper.getLevelsetValues();
}

}  // namespace plato::third_party_integration::krino
