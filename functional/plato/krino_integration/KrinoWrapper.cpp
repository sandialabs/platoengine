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
// #include <iomanip>
#include <stk_io/StkMeshIoBroker.hpp>
#include <stk_mesh/base/MetaData.hpp>
// #include <stk_util/diag/Timer.hpp>
#include <stk_util/environment/EnvData.hpp>

#include "KrinoWrapper.hpp"
#include "stk_mesh/base/Types.hpp"

namespace plato::krino_integration
{

void KrinoWrapper::writeMeshPrivate(const std::string &aFilename)
{
    stk::mesh::Selector outputSelector = buildOutputSelector(
        mBulkData->mesh_meta_data(), krino::AuxMetaData::get(mBulkData->mesh_meta_data()).active_part());
    krino::output_composed_mesh_with_fields(*mBulkData, outputSelector, aFilename, 1, 0.0);
}

stk::mesh::Selector KrinoWrapper::buildOutputSelector(const stk::mesh::MetaData &meta,
                                                      const stk::mesh::Part &activePart)
{
    stk::mesh::PartVector outputParts;
    for (auto *part : meta.get_parts())
    {
        if (stk::io::is_part_io_part(*part) && includeVoidRegionPart(part))
        {
            outputParts.push_back(part);
        }
    }
    return activePart & stk::mesh::selectUnion(outputParts);
}

void KrinoWrapper::readAndSetupMeshForDecompositionPrivate(const std::string &aFilename)
{
    mMeshFromFile = std::make_unique<krino::MeshFromFile>(aFilename, stk::EnvData::parallel_comm(), "rib");
    mLSFields = krino::LSPerInterfacePolicy::setup_levelsets_on_all_blocks_with_void_phase_for_any_negative_levelset(
        mMeshFromFile->meta_data(), 1);
    mLevelSet = &krino::LevelSet::build(mMeshFromFile->meta_data(), "LS", sierra::Diag::sierraTimer());
    mLevelSet->set_distance_name("LS");
    mLevelSet->setup();
    setupFieldsForConformingDecomposition(mMeshFromFile->meta_data());
    mMeshFromFile->populate_mesh();
    krino::activate_all_entities(mMeshFromFile->bulk_data(),
                                 krino::AuxMetaData::get(mMeshFromFile->meta_data()).active_part());
    mBulkData = &mMeshFromFile->bulk_data();
    stk::mesh::EntityVector nodes;
    stk::mesh::get_entities(*mBulkData, stk::topology::NODE_RANK, nodes);
    mUncutBackgroundMeshSize = nodes.size();
}

void KrinoWrapper::setupFieldsForConformingDecomposition(const stk::mesh::MetaData &meta)
{
    krino::CDFEM_Support &cdfemSupport = krino::CDFEM_Support::get(meta);
    const krino::FieldRef coordsField = meta.coordinate_field();

    cdfemSupport.set_coords_field(coordsField);
    cdfemSupport.add_edge_interpolation_field(coordsField);
    cdfemSupport.register_parent_node_ids_field();
}

bool KrinoWrapper::includeVoidRegionPart(const stk::mesh::Part *aPart)
{
    return (mIncludeVoidRegion || aPart->name().find("_void") == std::string::npos);
}

void KrinoWrapper::readAndSetupMeshForDecomposition(const std::string &aFilename)
{
    readAndSetupMeshForDecompositionPrivate(aFilename);
}

void KrinoWrapper::setLevelsetValues(const std::vector<double> &aValuesIn)
{
    krino::CDFEM_Support &cdfemSupport = krino::CDFEM_Support::get(mBulkData->mesh_meta_data());
    stk::mesh::Selector tSelector = stk::mesh::selectField(mLSFields[0].isovar) & !cdfemSupport.get_child_node_part();
    stk::mesh::EntityVector tNodes;
    stk::mesh::get_selected_entities(tSelector, mBulkData->buckets(stk::topology::NODE_RANK), tNodes);
    if (aValuesIn.size() != tNodes.size())
    {
        std::cout << "ERROR: Size mismatch when setting levelset values!" << std::endl;
        throw std::runtime_error("ERROR: Size mismatch when setting levelset values!");
    }
    for (size_t i = 0; i < tNodes.size(); ++i)
    {
        auto tCurNode = tNodes[i];
        double *dist = krino::field_data<double>(mLSFields[0].isovar, tCurNode);
        *dist = aValuesIn[i];
    }
}

void KrinoWrapper::cutMesh()
{
    decomposeMeshToConformToLevelsets(*mBulkData, mLSFields);
    mSensitivities = getLevelsetShapeSensitivities(*mBulkData, mLSFields[0].isovar);
}

void KrinoWrapper::decomposeMeshToConformToLevelsets(stk::mesh::BulkData &mesh,
                                                     const std::vector<krino::LS_Field> &lsFields)
{
    stk::mesh::MetaData &meta = mesh.mesh_meta_data();
    krino::AuxMetaData &auxMeta = krino::AuxMetaData::get(meta);
    krino::CDFEM_Support &cdfemSupport = krino::CDFEM_Support::get(meta);
    krino::Phase_Support &phaseSupport = krino::Phase_Support::get(meta);
    std::unique_ptr<krino::InterfaceGeometry> interfaceGeometry = krino::create_levelset_geometry(
        meta.spatial_dimension(), auxMeta.active_part(), cdfemSupport, phaseSupport, lsFields);
    auxMeta.clear_force_64bit_flag();
    krino::CDMesh::decompose_mesh(mesh, *interfaceGeometry);
}

std::map<stk::mesh::EntityId, InterfaceNode_DXDP> KrinoWrapper::getLevelsetShapeSensitivities(
    const stk::mesh::BulkData &mesh, const krino::FieldRef levelSetField)
{
    const krino::FieldRef coordsField = mesh.mesh_meta_data().coordinate_field();
    const krino::CDFEM_Support &cdfemSupport = krino::CDFEM_Support::get(mesh.mesh_meta_data());
    std::vector<krino::ChildNodeStencil> childNodeStencils;
    krino::fill_child_node_stencils(mesh, cdfemSupport.get_child_node_part(), cdfemSupport.get_parent_node_ids_field(),
                                    cdfemSupport.get_parent_node_weights_field(), childNodeStencils);

    std::vector<stk::mesh::EntityId> parentNodeIds;
    std::vector<stk::math::Vector3d> dCoordsdParentLevelSets;

    std::map<stk::mesh::EntityId, InterfaceNode_DXDP> tSensitivityMap;
    for (auto &stencil : childNodeStencils)
    {
        fillNodeIdsForNodes(mesh, stencil.parentNodes, parentNodeIds);
        fillDCoordsDLevelsets(coordsField, levelSetField, stencil.parentNodes,
                              /*stencil.parentWeights,*/ dCoordsdParentLevelSets);
        tSensitivityMap[mesh.identifier(stencil.childNode)] =
            InterfaceNode_DXDP{parentNodeIds, dCoordsdParentLevelSets};
    }

    return tSensitivityMap;
}

void KrinoWrapper::fillNodeIdsForNodes(const stk::mesh::BulkData &mesh,
                                       const std::vector<stk::mesh::Entity> &parentNodes,
                                       std::vector<stk::mesh::EntityId> &parentNodeIds)
{
    parentNodeIds.clear();
    for (auto parent : parentNodes) parentNodeIds.push_back(mesh.identifier(parent));
}

void KrinoWrapper::fillDCoordsDLevelsets(const krino::FieldRef coordsField,
                                         const krino::FieldRef levelSetField,
                                         const std::vector<stk::mesh::Entity> &parentNodes,
                                         std::vector<stk::math::Vector3d> &dCoordsdParentLevelSets)
{
    STK_ThrowRequireMsg(2 == parentNodes.size(), "Currently only edge intersections are supported.");
    dCoordsdParentLevelSets.clear();
    const stk::math::Vector3d x0(krino::field_data<double>(coordsField, parentNodes[0]));
    const stk::math::Vector3d x1(krino::field_data<double>(coordsField, parentNodes[1]));
    const double ls0 = *krino::field_data<double>(levelSetField, parentNodes[0]);
    const double ls1 = *krino::field_data<double>(levelSetField, parentNodes[1]);
    const stk::math::Vector3d dx = x1 - x0;
    const double sqrLo = (ls0 - ls1) * (ls0 - ls1);
    dCoordsdParentLevelSets.push_back((-ls1 / sqrLo) * dx);
    dCoordsdParentLevelSets.push_back((ls0 / sqrLo) * dx);
}

void KrinoWrapper::writeMesh(const std::string &aFilename) { writeMeshPrivate(aFilename); }

void KrinoWrapper::initializeLevelsetsFromPrimitives(const LevelsetPrimitives &aLevelsetPrimitives)
{
    initializeLevelsetFieldsFromPrimitives(mMeshFromFile->bulk_data(), mLSFields[0].isovar, aLevelsetPrimitives);
}

void KrinoWrapper::initializeLevelsetFieldsFromPrimitives(const stk::mesh::BulkData &mesh,
                                                          krino::FieldRef levelSetField,
                                                          const LevelsetPrimitives &aLevelsetPrimitives)
{
    const krino::FieldRef coordsField = mesh.mesh_meta_data().coordinate_field();
    krino::Composite_Surface initializationSurfaces("initialization surfaces");
    for (auto &sphere : aLevelsetPrimitives.mSpheres)
        initializationSurfaces.add(
            new krino::Sphere({sphere.mCenterX, sphere.mCenterY, sphere.mCenterZ}, sphere.mRadius));
    for (auto &plane : aLevelsetPrimitives.mPlanes)
    {
        const double tNormal[3] = {plane.mNormalX, plane.mNormalY, plane.mNormalZ};
        initializationSurfaces.add(new krino::Plane(tNormal, plane.mOffset, 1.0));
    }
    krino::compute_nodal_surface_distance(mesh, coordsField, levelSetField, initializationSurfaces);
}

std::vector<double> KrinoWrapper::getLevelsetValues()
{
    krino::CDFEM_Support &cdfemSupport = krino::CDFEM_Support::get(mBulkData->mesh_meta_data());
    stk::mesh::Selector tSelector = stk::mesh::selectField(mLSFields[0].isovar) & !cdfemSupport.get_child_node_part();
    stk::mesh::EntityVector tNodes;
    stk::mesh::get_selected_entities(tSelector, mBulkData->buckets(stk::topology::NODE_RANK), tNodes);

    std::vector<double> tReturn(tNodes.size());

    for (size_t i = 0; i < tNodes.size(); i++)
    {
        double *dist = krino::field_data<double>(mLSFields[0].isovar, tNodes[i]);
        tReturn[i] = *dist;
    }
    return tReturn;
}

void KrinoWrapper::createBoundingBoxMesh(const stk::math::Vector3d &aMinCorner,
                                         const stk::math::Vector3d &aMaxCorner,
                                         const double &aMeshSize,
                                         const std::string &aFilename)
{
    createBoundingBoxMeshPrivate(aMinCorner, aMaxCorner, aMeshSize, aFilename);
}

void KrinoWrapper::createBoundingBoxMeshPrivate(const stk::math::Vector3d &aMinCorner,
                                                const stk::math::Vector3d &aMaxCorner,
                                                const double &aMeshSize,
                                                const std::string &aFilename)
{
    mBoundingBoxMesh = std::make_unique<krino::BoundingBoxMesh>(stk::topology::TET_4, stk::EnvData::parallel_comm());
    mLSFields = krino::LSPerInterfacePolicy::setup_levelsets_on_all_blocks_with_void_phase_for_any_negative_levelset(
        mBoundingBoxMesh->meta_data(), 1);
    mLevelSet = &krino::LevelSet::build(mBoundingBoxMesh->meta_data(), "LS", sierra::Diag::sierraTimer());
    mLevelSet->set_distance_name("LS");
    mLevelSet->setup();
    setupFieldsForConformingDecomposition(mBoundingBoxMesh->meta_data());
    mBoundingBoxMesh->set_domain(krino::BoundingBoxMesh::BoundingBoxType(aMinCorner, aMaxCorner), aMeshSize);
    mBoundingBoxMesh->set_mesh_structure_type(krino::FLAT_WALLED_BCC_BOUNDING_BOX_MESH);
    mBoundingBoxMesh->populate_mesh();
    mBulkData = &mBoundingBoxMesh->bulk_data();
    krino::activate_all_entities(*mBulkData, krino::AuxMetaData::get(mBulkData->mesh_meta_data()).active_part());
    krino::output_composed_mesh_with_fields(
        *mBulkData, krino::AuxMetaData::get(mBoundingBoxMesh->meta_data()).active_part(), aFilename, 1, 0.0);
}

void KrinoWrapper::getNodalCoordinates(const unsigned int &aNodeID, double &aX, double &aY, double &aZ)
{
    const auto *tCoordsField =
        static_cast<const stk::mesh::Field<double> *>(mBulkData->mesh_meta_data().coordinate_field());
    stk::mesh::Entity entity = mBulkData->get_entity(stk::topology::NODE_RANK, aNodeID);
    double *vals = stk::mesh::field_data(*tCoordsField, entity);
    aX = vals[0];
    aY = vals[1];
    aZ = vals[2];
}

void KrinoWrapper::initializeSphereLevelset(const std::vector<std::pair<stk::math::Vector3d, double>> &aSpheres)
{
    initializeLevelsetFieldForSpheres(mMeshFromFile->bulk_data(), mLSFields[0].isovar, aSpheres);
}

void KrinoWrapper::initializeLevelsetFieldForSpheres(const stk::mesh::BulkData &mesh,
                                                     krino::FieldRef levelSetField,
                                                     const std::vector<std::pair<stk::math::Vector3d, double>> &spheres)
{
    const krino::FieldRef coordsField = mesh.mesh_meta_data().coordinate_field();
    krino::Composite_Surface initializationSurfaces("initialization surfaces");
    for (auto &sphere : spheres) initializationSurfaces.add(new krino::Sphere(sphere.first, sphere.second));
    compute_nodal_surface_distance(mesh, coordsField, levelSetField, initializationSurfaces);
}

unsigned int KrinoWrapper::getNumTetsInNamedBlock(const std::string &aBlockName)
{
    unsigned int tTotalSize = 0;
    stk::mesh::Part *tPart = mBulkData->mesh_meta_data().get_part(aBlockName);
    stk::mesh::Selector tSelector(*tPart);
    stk::mesh::BucketVector const &tBuckets = mBulkData->get_buckets(stk::topology::ELEM_RANK, tSelector);
    for (stk::mesh::BucketVector::const_iterator b_itr = tBuckets.begin(); b_itr != tBuckets.end(); ++b_itr)
    {
        stk::mesh::Bucket &b = **b_itr;
        tTotalSize += b.size();
    }
    return tTotalSize;
}

void KrinoWrapper::initializePlaneLevelset(const double &aNormalX,
                                           const double &aNormalY,
                                           const double &aNormalZ,
                                           const double &aOffset)
{
    initializeLevelsetFieldForPlane(*mBulkData, mLSFields[0].isovar, {aNormalX, aNormalY, aNormalZ}, aOffset);
}

void KrinoWrapper::initializeLevelsetFieldForPlane(const stk::mesh::BulkData &mesh,
                                                   krino::FieldRef levelSetField,
                                                   const stk::math::Vector3d &normal,
                                                   const double offset)
{
    const krino::FieldRef coordsField = mesh.mesh_meta_data().coordinate_field();
    krino::Composite_Surface initializationSurfaces("initialization surfaces");
    initializationSurfaces.add(new krino::Plane(normal.data(), offset, 1.0));
    compute_nodal_surface_distance(mesh, coordsField, levelSetField, initializationSurfaces);
}

void KrinoWrapper::resetMesh() { krino::CDMesh::reset_mesh_to_original_undecomposed_state(*mBulkData); }

void KrinoWrapper::redistance() { mLevelSet->redistance(); }

std::unordered_map<unsigned int, stk::math::Vector3d> KrinoWrapper::getCoordinateValues()
{
    std::unordered_map<unsigned int, stk::math::Vector3d> tCurCoordinateValues;
    const unsigned tNumDimensions = mBulkData->mesh_meta_data().spatial_dimension();
    stk::mesh::BucketVector const &tNodeBuckets =
        mBulkData->get_buckets(stk::topology::NODE_RANK, stk::mesh::selectField(mLSFields[0].isovar));
    const krino::FieldRef tCoordsField = mBulkData->mesh_meta_data().coordinate_field();
    for (auto &&tCurBucketPtr : tNodeBuckets)
    {
        const stk::mesh::Bucket &tCurBucket = *tCurBucketPtr;
        double *tBucketCoords = krino::field_data<double>(tCoordsField, tCurBucket);
        unsigned int tCntr = 0;
        for (stk::mesh::Entity tCurNodeEntity : tCurBucket)
        {
            unsigned int tGlobalNodeID = mBulkData->identifier(tCurNodeEntity);
            const stk::math::Vector3d tNodeCoords(&tBucketCoords[tNumDimensions * tCntr], tNumDimensions);
            tCurCoordinateValues[tGlobalNodeID] = tNodeCoords;
            tCntr++;
        }
    }
    return tCurCoordinateValues;
}

std::unordered_map<unsigned int, stk::math::Vector3d> KrinoWrapper::predictNewCoordinatesBasedOnPerturbedLevelsetValues(
    std::unordered_map<unsigned int, stk::math::Vector3d> &aCoordVals, const double &aPerturbation)
{
    std::unordered_map<unsigned int, stk::math::Vector3d> tPredictedCoordinateValues;
    std::map<stk::mesh::EntityId, InterfaceNode_DXDP>::iterator tMapIter = mSensitivities.begin();
    while (tMapIter != mSensitivities.end())
    {
        unsigned int tInterfaceNodeID = tMapIter->first;
        double dCoord[3] = {0, 0, 0};
        for (size_t i = 0; i < tMapIter->second.parentNodeIds.size(); ++i)
        {
            for (size_t j = 0; j < 3; ++j)
            {
                dCoord[j] += aPerturbation * tMapIter->second.parentDXDP[i][j];
            }
        }
        for (size_t j = 0; j < 3; ++j)
        {
            tPredictedCoordinateValues[tInterfaceNodeID][j] = aCoordVals[tInterfaceNodeID][j] + dCoord[j];
        }
        tMapIter++;
    }
    // now add coords for nodes that weren't on the interface (ones
    // we don't have sensitivities for)
    for (auto &tCoordValue : aCoordVals)
    {
        if (tPredictedCoordinateValues.count(tCoordValue.first) == 0)
        {
            tPredictedCoordinateValues[tCoordValue.first] = tCoordValue.second;
        }
    }
    return tPredictedCoordinateValues;
}

}  // namespace plato::krino_integration
