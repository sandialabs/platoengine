#include "FreeFunctionInterface.hpp"

#include "KrinoWrapper.hpp"
/*
#include <Akri_AnalyticSurf.hpp>
#include <Akri_AuxMetaData.hpp>
#include <Akri_CDFEM_Support.hpp>
#include <Akri_CDMesh.hpp>
#include <Akri_ChildNodeStencil.hpp>
#include <Akri_Composite_Surface.hpp>
#include <Akri_CreateInterfaceGeometry.hpp>
#include <Akri_MeshHelpers.hpp>
#include <Akri_NodalSurfaceDistance.hpp>
#include <Akri_OutputUtils.hpp>
#include <iomanip>
#include <stk_io/StkMeshIoBroker.hpp>
#include <stk_util/diag/Timer.hpp>
#include <stk_util/environment/EnvData.hpp>
*/

namespace plato::krino_integration
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

}  // namespace plato::krino_integration
