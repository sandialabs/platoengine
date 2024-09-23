#include "Interface.hpp"

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

#include "KrinoWrapper.hpp"

namespace plato::krino_integration
{

#if 0


std::map<stk::mesh::EntityId, InterfaceNode_DXDP> PlatoKrinoInterface::generateComputationalMesh(const std::string &aBackgroundMeshName,
                                   const std::string &aCutMesh,
                                   const std::vector<double> &aLevelsetValues,
                                   const bool aIncludeVoidRegion)
{
    includeVoidRegion(aIncludeVoidRegion);
    readAndSetupMeshForDecomposition(aBackgroundMeshName);
    setLevelsetValues(aLevelsetValues);
    cutMesh();
    storeSensitivities();
    writeMesh(aCutMesh);
    return mSensitivities;
}





/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// These functions all need to be provided through the krino namespace.
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////















/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// These functions will be called by the PlatoKrinoApp
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////












void PlatoKrinoInterface::setSensitivities(
    const std::vector<std::pair<unsigned int, std::vector<std::pair<unsigned int, stk::math::Vector3d>>>>
        &aSensitivities)
{
    mSensitivities.clear();
    for (auto tCurSens : aSensitivities)
    {
        std::vector<stk::mesh::EntityId> tParentIds;
        std::vector<stk::math::Vector3d> tParentLevelSets;
        for (auto &tCurParent : tCurSens.second)
        {
            tParentIds.push_back(tCurParent.first);
            tParentLevelSets.push_back(tCurParent.second);
        }
        mSensitivities[tCurSens.first] = InterfaceNode_DXDP{tParentIds, tParentLevelSets};
    }
}





void PlatoKrinoInterface::setLevelsetValues_parallel(const std::vector<double> &aValuesIn)
{
    krino::CDFEM_Support &cdfemSupport = krino::CDFEM_Support::get(mBulkData->mesh_meta_data());
    stk::mesh::Selector tSelector = stk::mesh::selectField(mLSFields[0].isovar) & !cdfemSupport.get_child_node_part();
    stk::mesh::EntityVector tNodes;
    stk::mesh::get_selected_entities(tSelector, mBulkData->buckets(stk::topology::NODE_RANK), tNodes);
    for (size_t i = 0; i < tNodes.size(); ++i)
    {
        auto tCurNode = tNodes[i];
        unsigned int tGlobalNodeID = mBulkData->identifier(tCurNode);
        double *dist = krino::field_data<double>(mLSFields[0].isovar, tCurNode);
        *dist = aValuesIn[tGlobalNodeID - 1];
    }
}


std::map<unsigned int, double> PlatoKrinoInterface::calculateDFDLS(std::map<unsigned int, stk::math::Vector3d> &aDFDX)
{
    std::map<unsigned int, double> tDFDLS;
    for (unsigned int i = 0; i < mUncutBackgroundMeshSize; ++i)
    {
        tDFDLS[i + 1] = 0.0;
    }
    std::map<stk::mesh::EntityId, InterfaceNode_DXDP>::iterator tMapIter = mSensitivities.begin();
    while (tMapIter != mSensitivities.end())
    {
        unsigned int tCurInterfaceNodeID = tMapIter->first;
        if (aDFDX.count(tCurInterfaceNodeID) == 0)
        {
            std::cout << "ERROR: Cut mesh interface global node id does not have a corresponding DFDX entry!"
                      << std::endl;
            throw 1;
        }

        for (size_t j = 0; j < tMapIter->second.parentNodeIds.size(); ++j)
        {
            unsigned int tCurBackgroundMeshNodeID = tMapIter->second.parentNodeIds[j];
            double tContribution = 0.0;
            for (size_t w = 0; w < 3; ++w)
            {
                tContribution += aDFDX[tCurInterfaceNodeID][w] * tMapIter->second.parentDXDP[j][w];
            }
            if (tDFDLS.count(tCurBackgroundMeshNodeID))
            {
                tDFDLS[tCurBackgroundMeshNodeID] += tContribution;
            }
            else
            {
                tDFDLS[tCurBackgroundMeshNodeID] = tContribution;
            }
        }
        tMapIter++;
    }
    return tDFDLS;
}

#endif

}  // namespace plato::krino_integration
