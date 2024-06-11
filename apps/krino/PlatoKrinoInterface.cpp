#include <iomanip>
#include "PlatoKrinoInterface.hpp"
#include <Akri_MeshHelpers.hpp>
#include <Akri_AuxMetaData.hpp>
#include <Akri_CDFEM_Support.hpp>
#include <Akri_Composite_Surface.hpp>
#include <Akri_AnalyticSurf.hpp>
#include <Akri_NodalSurfaceDistance.hpp>
#include <Akri_CreateInterfaceGeometry.hpp>
#include <Akri_CDMesh.hpp>
#include <Akri_OutputUtils.hpp>
#include <Akri_ChildNodeStencil.hpp>
#include <stk_io/StkMeshIoBroker.hpp>
#include <stk_util/environment/EnvData.hpp>
#include <stk_util/diag/Timer.hpp>

namespace Plato::Krino
{

std::map<stk::mesh::EntityId, InterfaceNode_DXDP> PlatoKrinoInterface::cut_mesh_and_return_sensitivities(const std::string &aBackgroundMeshName,
                  const std::string &aCutMesh, const std::vector<double> &aLevelsetValues)
{
    readAndSetupMeshForDecomposition(aBackgroundMeshName); 
    setLevelsetValues(aLevelsetValues);
    cutMesh();
    getSensitivities();
    writeMesh(aCutMesh);
    return mSensitivities;
}

std::vector<double> PlatoKrinoInterface::initialize_mesh_with_levelset_primitives_and_return_levelset_values(
                  const std::string &aBackgroundMeshName,
                  const std::string &aCutMesh, 
                  const LevelsetPrimitives &aLevelsetPrimitives)
{
    readAndSetupMeshForDecomposition(aBackgroundMeshName); 
    initializeLevelsetsFromPrimitives(aLevelsetPrimitives);
    cutMesh();
    writeMesh(aCutMesh);
    return getLevelsetValues();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// These functions all need to be provided through the krino namespace.
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void PlatoKrinoInterface::generate_bounding_box_mesh(krino::BoundingBoxMesh & bboxMesh, const stk::math::Vector3d & minCorner, const stk::math::Vector3d & maxCorner, const double meshSize)
{
  bboxMesh.set_domain(krino::BoundingBoxMesh::BoundingBoxType(minCorner, maxCorner), meshSize);
  bboxMesh.set_mesh_structure_type(krino::FLAT_WALLED_BCC_BOUNDING_BOX_MESH);
  bboxMesh.populate_mesh();
  stk::mesh::BulkData & mesh = bboxMesh.bulk_data();
  krino::activate_all_entities(mesh, krino::AuxMetaData::get(mesh.mesh_meta_data()).active_part());
}

void PlatoKrinoInterface::setup_fields_for_conforming_decomposition(const stk::mesh::MetaData & meta)
{
  krino::CDFEM_Support & cdfemSupport = krino::CDFEM_Support::get(meta);
  const krino::FieldRef coordsField = meta.coordinate_field();

  cdfemSupport.set_coords_field(coordsField);
  cdfemSupport.add_edge_interpolation_field(coordsField);
  cdfemSupport.register_parent_node_ids_field();
}

void PlatoKrinoInterface::fill_node_ids_for_nodes(const stk::mesh::BulkData & mesh, const std::vector<stk::mesh::Entity> & parentNodes, std::vector<stk::mesh::EntityId> & parentNodeIds)
{
  parentNodeIds.clear();
  for (auto parent : parentNodes)
    parentNodeIds.push_back(mesh.identifier(parent));
}

void PlatoKrinoInterface::fill_d_coords_d_levelsets(const krino::FieldRef coordsField, const krino::FieldRef levelSetField, const std::vector<stk::mesh::Entity> & parentNodes, /*const std::vector<double> & parentWeights,*/ std::vector<stk::math::Vector3d> & dCoordsdParentLevelSets)
{
  STK_ThrowRequireMsg(2 == parentNodes.size(), "Currently only edge intersections are supported.");
  dCoordsdParentLevelSets.clear();
  const stk::math::Vector3d x0(krino::field_data<double>(coordsField, parentNodes[0]));
  const stk::math::Vector3d x1(krino::field_data<double>(coordsField, parentNodes[1]));
  const double ls0 = *krino::field_data<double>(levelSetField, parentNodes[0]);
  const double ls1 = *krino::field_data<double>(levelSetField, parentNodes[1]);
  const stk::math::Vector3d dx = x1-x0;
  const double sqrLo = (ls0-ls1)*(ls0-ls1);
  dCoordsdParentLevelSets.push_back((-ls1/sqrLo) * dx);
  dCoordsdParentLevelSets.push_back((ls0/sqrLo) * dx);
}

bool PlatoKrinoInterface::includeVoidRegionPart(const stk::mesh::Part* aPart)
{
    return (mIncludeVoidRegion || aPart->name().find("_void") == std::string::npos);
}

stk::mesh::Selector PlatoKrinoInterface::build_output_selector(const stk::mesh::MetaData & meta, const stk::mesh::Part & activePart)
{
    stk::mesh::PartVector outputParts;
    for (auto * part : meta.get_parts())
    {
        if (stk::io::is_part_io_part(*part) && includeVoidRegionPart(part))
        {
            outputParts.push_back(part);
        }
    }
    return activePart & stk::mesh::selectUnion(outputParts);
}

std::map<stk::mesh::EntityId, InterfaceNode_DXDP> PlatoKrinoInterface::get_levelset_shape_sensitivities(const stk::mesh::BulkData & mesh, const krino::FieldRef levelSetField)
{
  const krino::FieldRef coordsField = mesh.mesh_meta_data().coordinate_field();
  const krino::CDFEM_Support & cdfemSupport = krino::CDFEM_Support::get(mesh.mesh_meta_data());
  std::vector<krino::ChildNodeStencil> childNodeStencils;
  krino::fill_child_node_stencils(mesh, cdfemSupport.get_child_node_part(), cdfemSupport.get_parent_node_ids_field(), cdfemSupport.get_parent_node_weights_field(), childNodeStencils);

  std::vector<stk::mesh::EntityId> parentNodeIds;
  std::vector<stk::math::Vector3d> dCoordsdParentLevelSets;

  std::map<stk::mesh::EntityId, InterfaceNode_DXDP> tSensitivityMap;
  for (auto & stencil : childNodeStencils)
  {
    fill_node_ids_for_nodes(mesh, stencil.parentNodes, parentNodeIds);
    fill_d_coords_d_levelsets(coordsField, levelSetField, stencil.parentNodes, /*stencil.parentWeights,*/ dCoordsdParentLevelSets);
    tSensitivityMap[mesh.identifier(stencil.childNode)] = InterfaceNode_DXDP{parentNodeIds, dCoordsdParentLevelSets};
  }

  return tSensitivityMap;
}

void PlatoKrinoInterface::generate_and_write_bounding_box_mesh(const stk::topology elemTopology, 
                                          const stk::math::Vector3d & minCorner, 
                                          const stk::math::Vector3d & maxCorner, 
                                          const double meshSize, const std::string & filename)
{
  krino::BoundingBoxMesh bboxMesh(elemTopology, stk::EnvData::parallel_comm());
  generate_bounding_box_mesh(bboxMesh, minCorner, maxCorner, meshSize);
  krino::output_composed_mesh_with_fields(bboxMesh.bulk_data(), krino::AuxMetaData::get(bboxMesh.meta_data()).active_part(), filename, 1, 0.0);
}

void PlatoKrinoInterface::initialize_levelset_field_for_plane(const stk::mesh::BulkData & mesh, krino::FieldRef levelSetField, const stk::math::Vector3d & normal, const double offset)
{
  const krino::FieldRef coordsField = mesh.mesh_meta_data().coordinate_field();
  krino::Composite_Surface initializationSurfaces("initialization surfaces");
  initializationSurfaces.add(new krino::Plane(normal.data(), offset, 1.0));
  compute_nodal_surface_distance(mesh, coordsField, levelSetField, initializationSurfaces);
}

void PlatoKrinoInterface::initialize_levelset_field_for_spheres(const stk::mesh::BulkData & mesh, krino::FieldRef levelSetField, const std::vector<std::pair<stk::math::Vector3d,double>> & spheres)
{
  const krino::FieldRef coordsField = mesh.mesh_meta_data().coordinate_field();
  krino::Composite_Surface initializationSurfaces("initialization surfaces");
  for (auto & sphere : spheres)
    initializationSurfaces.add(new krino::Sphere(sphere.first, sphere.second));
  compute_nodal_surface_distance(mesh, coordsField, levelSetField, initializationSurfaces);
}

void PlatoKrinoInterface::initialize_levelset_fields_from_primitives(const stk::mesh::BulkData & mesh, krino::FieldRef levelSetField,
                                                                     const LevelsetPrimitives &aLevelsetPrimitives)
{
    const krino::FieldRef coordsField = mesh.mesh_meta_data().coordinate_field();
    krino::Composite_Surface initializationSurfaces("initialization surfaces");
    for (auto & sphere : aLevelsetPrimitives.mSpheres)
        initializationSurfaces.add(new krino::Sphere({sphere.mCenterX, sphere.mCenterY, sphere.mCenterZ}, sphere.mRadius));
    for (auto & plane : aLevelsetPrimitives.mPlanes)
    {
        const double tNormal[3] = {plane.mNormalX, plane.mNormalY, plane.mNormalZ};
        initializationSurfaces.add(new krino::Plane(tNormal, plane.mOffset, 1.0));
    }
    compute_nodal_surface_distance(mesh, coordsField, levelSetField, initializationSurfaces);
}

void PlatoKrinoInterface::read_and_setup_mesh_for_decomposition(const std::string &aFilename)
{
    mMeshFromFile = std::make_unique<krino::MeshFromFile>(aFilename, stk::EnvData::parallel_comm(), "rib");
    mLSFields = krino::LSPerInterfacePolicy::setup_levelsets_on_all_blocks_with_void_phase_for_any_negative_levelset(mMeshFromFile->meta_data(), 1);
    mLevelSet = &krino::LevelSet::build(mMeshFromFile->meta_data(), "LS", sierra::Diag::sierraTimer());
    mLevelSet->set_distance_name("LS");
    mLevelSet->setup();
    setup_fields_for_conforming_decomposition(mMeshFromFile->meta_data());
    mMeshFromFile->populate_mesh();
    krino::activate_all_entities(mMeshFromFile->bulk_data(), krino::AuxMetaData::get(mMeshFromFile->meta_data()).active_part());
    mBulkData = &mMeshFromFile->bulk_data();
    stk::mesh::EntityVector nodes;
    stk::mesh::get_entities(*mBulkData, stk::topology::NODE_RANK, nodes);
    mUncutBackgroundMeshSize = nodes.size();
}

void PlatoKrinoInterface::decompose_mesh_to_conform_to_levelsets(stk::mesh::BulkData & mesh, const std::vector<krino::LS_Field> & lsFields)
{
  stk::mesh::MetaData & meta = mesh.mesh_meta_data();
  krino::AuxMetaData & auxMeta = krino::AuxMetaData::get(meta);
  krino::CDFEM_Support & cdfemSupport = krino::CDFEM_Support::get(meta);
  krino::Phase_Support & phaseSupport = krino::Phase_Support::get(meta);
  std::unique_ptr<krino::InterfaceGeometry> interfaceGeometry = krino::create_levelset_geometry(meta.spatial_dimension(), auxMeta.active_part(), cdfemSupport, phaseSupport, lsFields);
  auxMeta.clear_force_64bit_flag();
  krino::CDMesh::decompose_mesh(mesh, *interfaceGeometry);
}

void PlatoKrinoInterface::create_bounding_box_mesh(const double &aXMin, const double &aYMin, const double &aZMin,
                                                   const double &aXMax, const double &aYMax, const double &aZMax, 
                                                   const double &aMeshSize)
{
    mBoundingBoxMesh = std::make_unique<krino::BoundingBoxMesh>(stk::topology::TET_4, stk::EnvData::parallel_comm()); 
    mLSFields = krino::LSPerInterfacePolicy::setup_levelsets_on_all_blocks_with_void_phase_for_any_negative_levelset(mBoundingBoxMesh->meta_data(), 1);
    mLevelSet = &krino::LevelSet::build(mBoundingBoxMesh->meta_data(), "LS", sierra::Diag::sierraTimer());
    mLevelSet->set_distance_name("LS");
    mLevelSet->setup();
    setup_fields_for_conforming_decomposition(mBoundingBoxMesh->meta_data());
    generate_bounding_box_mesh(*mBoundingBoxMesh, {aXMin,aYMin,aZMin}, {aXMax,aYMax,aZMax}, aMeshSize);
    mBulkData = &mBoundingBoxMesh->bulk_data();
}

void PlatoKrinoInterface::write_mesh(const std::string& aFilename)
{
    stk::mesh::Selector outputSelector = build_output_selector(mBulkData->mesh_meta_data(), krino::AuxMetaData::get(mBulkData->mesh_meta_data()).active_part());
    krino::output_composed_mesh_with_fields(*mBulkData, outputSelector, aFilename, 1, 0.0);
}



/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// These functions will be called by the PlatoKrinoApp
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void PlatoKrinoInterface::cutMesh()
{
    decompose_mesh_to_conform_to_levelsets(*mBulkData, mLSFields);
}

void PlatoKrinoInterface::initializePlaneLevelset(const double &aNormalX, const double &aNormalY, const double &aNormalZ, const double &aOffset)
{
    initialize_levelset_field_for_plane(*mBulkData, mLSFields[0].isovar, {aNormalX,aNormalY,aNormalZ}, aOffset);
}

void PlatoKrinoInterface::initializeLevelsetsFromPrimitives(const LevelsetPrimitives &aLevelsetPrimitives)
{
    initialize_levelset_fields_from_primitives(mMeshFromFile->bulk_data(), mLSFields[0].isovar, aLevelsetPrimitives);
}

void PlatoKrinoInterface::initializeSphereLevelset(const std::vector<std::pair<stk::math::Vector3d,double>> &aSpheres)
{
    initialize_levelset_field_for_spheres(mMeshFromFile->bulk_data(), mLSFields[0].isovar, aSpheres);
}

void PlatoKrinoInterface::readAndSetupMeshForDecomposition(const std::string &aFilename)
{
    read_and_setup_mesh_for_decomposition(aFilename);
}

void PlatoKrinoInterface::createBoundingBoxMesh(const double &aXMin, const double &aYMin, const double &aZMin,
                                                   const double &aXMax, const double &aYMax, const double &aZMax, 
                                                   const double &aMeshSize)
{
    create_bounding_box_mesh(aXMin, aYMin, aZMin, aXMax, aYMax, aZMax, aMeshSize);
}

void PlatoKrinoInterface::createAndWriteBoundingBoxMesh(const stk::math::Vector3d & aMinCorner,
                                                        const stk::math::Vector3d & aMaxCorner, 
                                                        const double &aMeshSize, const std::string &aFilename)
{
    generate_and_write_bounding_box_mesh(stk::topology::TET_4, aMinCorner, aMaxCorner, aMeshSize, aFilename);
}

void PlatoKrinoInterface::getSensitivities()
{
    mSensitivities = get_levelset_shape_sensitivities(*mBulkData, mLSFields[0].isovar);
}

void PlatoKrinoInterface::writeMesh(const std::string& aFilename)
{
    write_mesh(aFilename);
}

std::unordered_map<unsigned int, stk::math::Vector3d> PlatoKrinoInterface::predictNewCoordinatesBasedOnPerturbedLevelsetValues(
                            std::unordered_map<unsigned int, stk::math::Vector3d> &aCoordVals, const double &aPerturbation)
{
    std::unordered_map<unsigned int, stk::math::Vector3d> tPredictedCoordinateValues;
    std::map<stk::mesh::EntityId, InterfaceNode_DXDP>::iterator tMapIter = mSensitivities.begin();
    while (tMapIter != mSensitivities.end())
    {
        unsigned int tInterfaceNodeID = tMapIter->first;
        double dCoord[3] = {0,0,0};
        for(size_t i=0; i<tMapIter->second.parentNodeIds.size(); ++i)
        {
            for(size_t j=0; j<3; ++j)
            {
                dCoord[j] += aPerturbation*tMapIter->second.parentDXDP[i][j];
            }
        }
        for(size_t j=0; j<3; ++j)
        {
            tPredictedCoordinateValues[tInterfaceNodeID][j] = 
                       aCoordVals[tInterfaceNodeID][j]+dCoord[j];
        }
        tMapIter++;
    }
    // now add coords for nodes that weren't on the interface (ones
    // we don't have sensitivities for)
    for(auto &tCoordValue : aCoordVals)
    {
        if(tPredictedCoordinateValues.count(tCoordValue.first) == 0)
        {
            tPredictedCoordinateValues[tCoordValue.first] = tCoordValue.second;
        }
    }
    return tPredictedCoordinateValues;
}

std::map<unsigned int, double> PlatoKrinoInterface::calculateDFDLS(std::map<unsigned int, stk::math::Vector3d> &aDFDX)
{
    std::map<unsigned int, double> tDFDLS;
    for(unsigned int i=0; i<mUncutBackgroundMeshSize; ++i)
    {
        tDFDLS[i+1] = 0.0;
    }
    std::map<stk::mesh::EntityId, InterfaceNode_DXDP>::iterator tMapIter = mSensitivities.begin();
    while(tMapIter != mSensitivities.end()) 
    {
        unsigned int tCurInterfaceNodeID = tMapIter->first;
        if(aDFDX.count(tCurInterfaceNodeID) == 0)
        {
            std::cout << "ERROR: Cut mesh interface global node id does not have a corresponding DFDX entry!" << std::endl;
            throw 1;
        }

        for(size_t j=0; j<tMapIter->second.parentNodeIds.size(); ++j)
        {
            unsigned int tCurBackgroundMeshNodeID = tMapIter->second.parentNodeIds[j];
            double tContribution = 0.0;
            for(size_t w=0; w<3; ++w)
            {
                tContribution += aDFDX[tCurInterfaceNodeID][w] * tMapIter->second.parentDXDP[j][w];
            }
            if(tDFDLS.count(tCurBackgroundMeshNodeID))
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

void PlatoKrinoInterface::getNodalCoordinates(const unsigned int &aNodeID, double &aX, double &aY, double &aZ)
{
    const auto * tCoordsField = static_cast<const stk::mesh::Field<double>*>(mBulkData->mesh_meta_data().coordinate_field());
    stk::mesh::Entity entity = mBulkData->get_entity(stk::topology::NODE_RANK, aNodeID);
    double* vals = stk::mesh::field_data(*tCoordsField, entity);
    aX = vals[0];
    aY = vals[1];
    aZ = vals[2];
}

unsigned int PlatoKrinoInterface::getNumTetsInNamedBlock(const std::string &aBlockName)
{
    unsigned int tTotalSize=0;
    stk::mesh::Part *tPart = mBulkData->mesh_meta_data().get_part(aBlockName);
    stk::mesh::Selector tSelector(*tPart);
    stk::mesh::BucketVector const &tBuckets = mBulkData->get_buckets(stk::topology::ELEM_RANK, tSelector);
    for (stk::mesh::BucketVector::const_iterator b_itr = tBuckets.begin();
                        b_itr != tBuckets.end(); ++b_itr)
    {
        stk::mesh::Bucket & b = **b_itr;
        tTotalSize += b.size();
    }
    return tTotalSize;
}

void PlatoKrinoInterface::setSensitivities(const std::vector<std::pair<unsigned int, std::vector<std::pair<
                                                unsigned int, stk::math::Vector3d>>>> &aSensitivities)
{
    mSensitivities.clear();
    for(auto tCurSens : aSensitivities)
    {
        std::vector<stk::mesh::EntityId> tParentIds;
        std::vector<stk::math::Vector3d> tParentLevelSets;
        for(auto &tCurParent : tCurSens.second)
        {
            tParentIds.push_back(tCurParent.first);
            tParentLevelSets.push_back(tCurParent.second);
        }
        mSensitivities[tCurSens.first] = InterfaceNode_DXDP{tParentIds, tParentLevelSets};
    }
}

std::vector<double> PlatoKrinoInterface::getLevelsetValues()
{
    krino::CDFEM_Support & cdfemSupport = krino::CDFEM_Support::get(mBulkData->mesh_meta_data());
    stk::mesh::Selector tSelector = stk::mesh::selectField(mLSFields[0].isovar) & !cdfemSupport.get_child_node_part();
    stk::mesh::EntityVector tNodes;
    stk::mesh::get_selected_entities(tSelector, mBulkData->buckets(stk::topology::NODE_RANK), tNodes);

    std::vector<double> tReturn(tNodes.size());

    for(size_t i=0; i<tNodes.size(); i++)
    {
        double *dist = krino::field_data<double>(mLSFields[0].isovar, tNodes[i]);
        tReturn[i] = *dist;
    }
    return tReturn;
}

std::unordered_map<unsigned int, stk::math::Vector3d> PlatoKrinoInterface::getCoordinateValues()
{
    std::unordered_map<unsigned int, stk::math::Vector3d> tCurCoordinateValues;
    const unsigned tNumDimensions = mBulkData->mesh_meta_data().spatial_dimension();
    stk::mesh::BucketVector const& tNodeBuckets = mBulkData->get_buckets(stk::topology::NODE_RANK, 
                                      stk::mesh::selectField(mLSFields[0].isovar) );
    const krino::FieldRef tCoordsField = mBulkData->mesh_meta_data().coordinate_field();
    for ( auto && tCurBucketPtr : tNodeBuckets )
    {
        const stk::mesh::Bucket & tCurBucket = *tCurBucketPtr;
        double *tBucketCoords = krino::field_data<double>(tCoordsField, tCurBucket);
        unsigned int tCntr=0;
        for(stk::mesh::Entity tCurNodeEntity : tCurBucket)
        {
            unsigned int tGlobalNodeID = mBulkData->identifier(tCurNodeEntity);
            const stk::math::Vector3d tNodeCoords(&tBucketCoords[tNumDimensions*tCntr], tNumDimensions);
            tCurCoordinateValues[tGlobalNodeID] = tNodeCoords;
            tCntr++;
        }
    }
    return tCurCoordinateValues;
}

void PlatoKrinoInterface::resetMesh()
{
    krino::CDMesh::reset_mesh_to_original_undecomposed_state(*mBulkData);
}

void PlatoKrinoInterface::setLevelsetValues(const std::vector<double> &aValuesIn)
{
    krino::CDFEM_Support & cdfemSupport = krino::CDFEM_Support::get(mBulkData->mesh_meta_data());
    stk::mesh::Selector tSelector = stk::mesh::selectField(mLSFields[0].isovar) & !cdfemSupport.get_child_node_part();
    stk::mesh::EntityVector tNodes;
    stk::mesh::get_selected_entities(tSelector, mBulkData->buckets(stk::topology::NODE_RANK), tNodes);
    if(aValuesIn.size() != tNodes.size())
    {
        std::cout << "ERROR: Size mismatch when setting levelset values!" << std::endl;
        throw std::runtime_error("ERROR: Size mismatch when setting levelset values!");
    }
    for(size_t i=0; i<tNodes.size(); ++i)
    {
        auto tCurNode = tNodes[i];
        double *dist = krino::field_data<double>(mLSFields[0].isovar, tCurNode);
        *dist = aValuesIn[i];
    }
}

void PlatoKrinoInterface::setLevelsetValues_parallel(const std::vector<double> &aValuesIn)
{
    krino::CDFEM_Support & cdfemSupport = krino::CDFEM_Support::get(mBulkData->mesh_meta_data());
    stk::mesh::Selector tSelector = stk::mesh::selectField(mLSFields[0].isovar) & !cdfemSupport.get_child_node_part();
    stk::mesh::EntityVector tNodes;
    stk::mesh::get_selected_entities(tSelector, mBulkData->buckets(stk::topology::NODE_RANK), tNodes);
    for(size_t i=0; i<tNodes.size(); ++i)
    {
        auto tCurNode = tNodes[i];
        unsigned int tGlobalNodeID = mBulkData->identifier(tCurNode);
        double *dist = krino::field_data<double>(mLSFields[0].isovar, tCurNode);
        *dist = aValuesIn[tGlobalNodeID-1];
    }
}

void PlatoKrinoInterface::redistance()
{
    mLevelSet->redistance();
}


}



