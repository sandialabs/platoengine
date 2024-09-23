#include "Interface.hpp"

#include "KrinoWrapper.hpp"

namespace plato::third_party_integration::krino
{
std::map<stk::mesh::EntityId, InterfaceNode_DXDP> generateComputationalMesh(const std::string &aBackgroundMeshName,
                                                                            const std::string &aCutMesh,
                                                                            const std::vector<double> &aLevelsetValues,
                                                                            const bool aIncludeVoidRegion)
{
    KrinoWrapper tKrinoWrapper;
    tKrinoWrapper.setIncludeVoidRegion(aIncludeVoidRegion);
    tKrinoWrapper.readAndSetupMeshForDecomposition(aBackgroundMeshName);
    tKrinoWrapper.setLevelsetValues(aLevelsetValues);
    tKrinoWrapper.cutMesh();
    tKrinoWrapper.writeMesh(aCutMesh);
    return tKrinoWrapper.getSensitivities();
}

std::vector<double> initializeMeshWithLevelsetPrimitives(const std::string &aBackgroundMeshName,
                                                         const std::string &aCutMesh,
                                                         const LevelsetPrimitives &aLevelsetPrimitives,
                                                         const bool aIncludeVoidRegion)
{
    KrinoWrapper tKrinoWrapper;
    tKrinoWrapper.setIncludeVoidRegion(aIncludeVoidRegion);
    tKrinoWrapper.readAndSetupMeshForDecomposition(aBackgroundMeshName);
    tKrinoWrapper.initializeLevelsetsFromPrimitives(aLevelsetPrimitives);
    tKrinoWrapper.cutMesh();
    tKrinoWrapper.writeMesh(aCutMesh);
    return tKrinoWrapper.getLevelsetValues();
}

}  // namespace plato::third_party_integration::krino
