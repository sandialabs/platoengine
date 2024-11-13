#include "plato/third_party_integration/krino/KrinoWrapper.hpp"

#include <Akri_AnalyticSurf.hpp>
#include <Akri_AuxMetaData.hpp>
#include <Akri_BoundingBoxMesh.hpp>
#include <Akri_CDFEM_Support.hpp>
#include <Akri_CDMesh.hpp>
#include <Akri_ChildNodeStencil.hpp>
#include <Akri_Composite_Surface.hpp>
#include <Akri_CreateInterfaceGeometry.hpp>
#include <Akri_MeshFromFile.hpp>
#include <Akri_MeshHelpers.hpp>
#include <Akri_NodalSurfaceDistance.hpp>
#include <Akri_OutputUtils.hpp>
#include <Akri_Surface_Manager.hpp>
#include <stk_io/StkMeshIoBroker.hpp>
#include <stk_mesh/base/MetaData.hpp>
#include <stk_mesh/base/Types.hpp>
#include <stk_util/environment/EnvData.hpp>

namespace plato::third_party_integration::krino
{
namespace
{
const std::string kLevelsetName = "LS";
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

void KrinoWrapper::setupFieldsForConformingDecomposition(const stk::mesh::MetaData &meta)
{
    ::krino::CDFEM_Support &cdfemSupport = ::krino::CDFEM_Support::get(meta);
    const ::krino::FieldRef coordsField = meta.coordinate_field();

    cdfemSupport.set_coords_field(coordsField);
    cdfemSupport.add_edge_interpolation_field(coordsField);
    cdfemSupport.register_parent_node_ids_field();
}

bool KrinoWrapper::includeVoidRegionPart(const stk::mesh::Part *aPart)
{
    return (mIncludeVoidRegion || aPart->name().find("_void") == std::string::npos);
}

void KrinoWrapper::setLevelsetValues(const std::vector<double> &aValuesIn)
{
    stk::mesh::EntityVector tNodes = getNodeEntitiesInMesh();
    if (aValuesIn.size() != tNodes.size())
    {
        throw std::runtime_error("ERROR: Size mismatch when setting levelset values!");
    }
    for (size_t i = 0; i < tNodes.size(); ++i)
    {
        const auto tCurNode = tNodes[i];
        double *dist = ::krino::field_data<double>(mLSFields[0].isovar, tCurNode);
        *dist = aValuesIn[i];
    }
}

void KrinoWrapper::cutMesh()
{
    decomposeMeshToConformToLevelsets(mKrinoMesh->bulk_data(), mLSFields);
    mSensitivities = getLevelsetShapeSensitivities(mKrinoMesh->bulk_data(), mLSFields[0].isovar);
}

void KrinoWrapper::decomposeMeshToConformToLevelsets(stk::mesh::BulkData &mesh,
                                                     const std::vector<::krino::LS_Field> &lsFields)
{
    stk::mesh::MetaData &meta = mesh.mesh_meta_data();
    ::krino::AuxMetaData &auxMeta = ::krino::AuxMetaData::get(meta);
    ::krino::CDFEM_Support &cdfemSupport = ::krino::CDFEM_Support::get(meta);
    ::krino::Phase_Support &phaseSupport = ::krino::Phase_Support::get(meta);
    std::unique_ptr<::krino::InterfaceGeometry> interfaceGeometry = ::krino::create_levelset_geometry(
        static_cast<int>(meta.spatial_dimension()), auxMeta.active_part(), cdfemSupport, phaseSupport, lsFields);
    auxMeta.clear_force_64bit_flag();
    ::krino::CDMesh::decompose_mesh(mesh, *interfaceGeometry);
}

std::unordered_map<stk::mesh::EntityId, InterfaceNodeDXDP> KrinoWrapper::getLevelsetShapeSensitivities(
    const stk::mesh::BulkData &mesh, const ::krino::FieldRef levelSetField)
{
    const ::krino::FieldRef coordsField = mesh.mesh_meta_data().coordinate_field();
    const ::krino::CDFEM_Support &cdfemSupport = ::krino::CDFEM_Support::get(mesh.mesh_meta_data());
    std::vector<::krino::ChildNodeStencil> childNodeStencils;
    ::krino::fill_child_node_stencils(mesh, cdfemSupport.get_child_node_part(),
                                      cdfemSupport.get_parent_node_ids_field(),
                                      cdfemSupport.get_parent_node_weights_field(), childNodeStencils);

    std::vector<stk::mesh::EntityId> parentNodeIds;
    std::vector<stk::math::Vector3d> dCoordsdParentLevelSets;

    std::unordered_map<stk::mesh::EntityId, InterfaceNodeDXDP> tSensitivityMap;
    for (auto &stencil : childNodeStencils)
    {
        fillNodeIdsForNodes(mesh, stencil.parentNodes, parentNodeIds);
        fillDCoordsDLevelsets(coordsField, levelSetField, stencil.parentNodes,
                              /*stencil.parentWeights,*/ dCoordsdParentLevelSets);
        tSensitivityMap[mesh.identifier(stencil.childNode)] = InterfaceNodeDXDP{parentNodeIds, dCoordsdParentLevelSets};
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

void KrinoWrapper::fillDCoordsDLevelsets(const ::krino::FieldRef coordsField,
                                         const ::krino::FieldRef levelSetField,
                                         const std::vector<stk::mesh::Entity> &parentNodes,
                                         std::vector<stk::math::Vector3d> &dCoordsdParentLevelSets)
{
    STK_ThrowRequireMsg(2 == parentNodes.size(), "Currently only edge intersections are supported.");
    dCoordsdParentLevelSets.clear();
    const stk::math::Vector3d x0(::krino::field_data<double>(coordsField, parentNodes[0]));
    const stk::math::Vector3d x1(::krino::field_data<double>(coordsField, parentNodes[1]));
    const double ls0 = *::krino::field_data<double>(levelSetField, parentNodes[0]);
    const double ls1 = *::krino::field_data<double>(levelSetField, parentNodes[1]);
    const stk::math::Vector3d dx = x1 - x0;
    const double sqrLo = (ls0 - ls1) * (ls0 - ls1);
    dCoordsdParentLevelSets.push_back((-ls1 / sqrLo) * dx);
    dCoordsdParentLevelSets.push_back((ls0 / sqrLo) * dx);
}

void KrinoWrapper::writeMesh(const std::filesystem::path &aFilename)
{
    const stk::mesh::Selector outputSelector =
        buildOutputSelector(mKrinoMesh->meta_data(), ::krino::AuxMetaData::get(mKrinoMesh->meta_data()).active_part());
    ::krino::output_composed_mesh_with_fields(mKrinoMesh->bulk_data(), outputSelector, aFilename.string(), 1, 0.0);
}

void KrinoWrapper::initializeLevelsetsFromPrimitives(const LevelsetPrimitives &aLevelsetPrimitives)
{
    initializeLevelsetFieldsFromPrimitives(mKrinoMesh->bulk_data(), mLSFields[0].isovar, aLevelsetPrimitives);
}

void KrinoWrapper::initializeLevelsetFieldsFromPrimitives(const stk::mesh::BulkData &mesh,
                                                          ::krino::FieldRef levelSetField,
                                                          const LevelsetPrimitives &aLevelsetPrimitives)
{
    const ::krino::FieldRef coordsField = mesh.mesh_meta_data().coordinate_field();
    ::krino::Composite_Surface initializationSurfaces("initialization surfaces");
    for (auto &sphere : aLevelsetPrimitives.mSpheres)
        initializationSurfaces.add(
            new ::krino::Sphere({sphere.mCenter.x, sphere.mCenter.y, sphere.mCenter.z}, sphere.mRadius));
    for (auto &plane : aLevelsetPrimitives.mPlanes)
    {
        const double tNormal[3] = {plane.mNormal.x, plane.mNormal.y, plane.mNormal.z};
        initializationSurfaces.add(new ::krino::Plane(tNormal, plane.mOffset, 1.0));
    }
    ::krino::compute_nodal_surface_distance(mesh, coordsField, levelSetField, initializationSurfaces);
}

stk::mesh::EntityVector KrinoWrapper::getNodeEntitiesInMesh() const
{
    ::krino::CDFEM_Support &cdfemSupport = ::krino::CDFEM_Support::get(mKrinoMesh->meta_data());
    const stk::mesh::Selector tSelector =
        stk::mesh::selectField(mLSFields[0].isovar) & !cdfemSupport.get_child_node_part();
    stk::mesh::EntityVector tNodes;
    stk::mesh::get_selected_entities(tSelector, mKrinoMesh->bulk_data().buckets(stk::topology::NODE_RANK), tNodes);
    return tNodes;
}

std::vector<double> KrinoWrapper::getLevelsetValues() const
{
    stk::mesh::EntityVector tNodes = getNodeEntitiesInMesh();
    std::vector<double> tReturn(tNodes.size());
    for (size_t i = 0; i < tNodes.size(); i++)
    {
        const double *dist = ::krino::field_data<double>(mLSFields[0].isovar, tNodes[i]);
        tReturn[i] = *dist;
    }
    return tReturn;
}

KrinoWrapper::KrinoWrapper(const stk::math::Vector3d &aMinCorner,
                           const stk::math::Vector3d &aMaxCorner,
                           const double aMeshSize,
                           const std::filesystem::path &aFilename,
                           const bool aIncludeVoidRegion)
    : mUncutBackgroundMeshSize(0),
      mIncludeVoidRegion(aIncludeVoidRegion),
      mLSFields(),
      mKrinoMesh(createBoundingBoxMesh(aMinCorner, aMaxCorner, aMeshSize, aFilename))
{
}

KrinoWrapper::KrinoWrapper(const std::filesystem::path &aFilename, const bool aIncludeVoidRegion)
    : mUncutBackgroundMeshSize(0),
      mIncludeVoidRegion(aIncludeVoidRegion),
      mLSFields(),
      mKrinoMesh(readAndSetupMeshForDecomposition(aFilename))
{
}

std::unique_ptr<::krino::MeshInterface> KrinoWrapper::createBoundingBoxMesh(const stk::math::Vector3d &aMinCorner,
                                                                            const stk::math::Vector3d &aMaxCorner,
                                                                            const double aMeshSize,
                                                                            const std::filesystem::path &aFilename)
{
    std::unique_ptr<::krino::BoundingBoxMesh> tBoundingBoxMesh =
        std::make_unique<::krino::BoundingBoxMesh>(stk::topology::TET_4, stk::EnvData::parallel_comm());
    mLSFields = ::krino::LSPerInterfacePolicy::setup_levelsets_on_all_blocks_with_void_phase_for_any_negative_levelset(
        tBoundingBoxMesh->meta_data(), 1);
    ::krino::LevelSet &tLevelSet =
        ::krino::LevelSet::build(tBoundingBoxMesh->meta_data(), kLevelsetName, sierra::Diag::sierraTimer());
    tLevelSet.set_distance_name(kLevelsetName);
    tLevelSet.setup();
    setupFieldsForConformingDecomposition(tBoundingBoxMesh->meta_data());
    tBoundingBoxMesh->set_domain(::krino::BoundingBoxMesh::BoundingBoxType(aMinCorner, aMaxCorner), aMeshSize);
    tBoundingBoxMesh->set_mesh_structure_type(::krino::FLAT_WALLED_BCC_BOUNDING_BOX_MESH);
    tBoundingBoxMesh->populate_mesh();
    ::krino::activate_all_entities(tBoundingBoxMesh->bulk_data(),
                                   ::krino::AuxMetaData::get(tBoundingBoxMesh->meta_data()).active_part());
    ::krino::output_composed_mesh_with_fields(tBoundingBoxMesh->bulk_data(),
                                              ::krino::AuxMetaData::get(tBoundingBoxMesh->meta_data()).active_part(),
                                              aFilename.string(), 1, 0.0);
    return std::unique_ptr<::krino::MeshInterface>(std::move(tBoundingBoxMesh));
}

std::unique_ptr<::krino::MeshInterface> KrinoWrapper::readAndSetupMeshForDecomposition(
    const std::filesystem::path &aFilename)
{
    std::unique_ptr<::krino::MeshFromFile> tMeshFromFile =
        std::make_unique<::krino::MeshFromFile>(aFilename.string(), stk::EnvData::parallel_comm(), "rib");
    mLSFields = ::krino::LSPerInterfacePolicy::setup_levelsets_on_all_blocks_with_void_phase_for_any_negative_levelset(
        tMeshFromFile->meta_data(), 1);
    ::krino::LevelSet &tLevelSet =
        ::krino::LevelSet::build(tMeshFromFile->meta_data(), kLevelsetName, sierra::Diag::sierraTimer());
    tLevelSet.set_distance_name(kLevelsetName);
    tLevelSet.setup();
    setupFieldsForConformingDecomposition(tMeshFromFile->meta_data());
    tMeshFromFile->populate_mesh();
    ::krino::activate_all_entities(tMeshFromFile->bulk_data(),
                                   ::krino::AuxMetaData::get(tMeshFromFile->meta_data()).active_part());
    stk::mesh::EntityVector nodes;
    stk::mesh::get_entities(tMeshFromFile->bulk_data(), stk::topology::NODE_RANK, nodes);
    mUncutBackgroundMeshSize = nodes.size();
    return std::unique_ptr<::krino::MeshInterface>(std::move(tMeshFromFile));
}

void KrinoWrapper::initializeSphereLevelset(const std::vector<std::pair<stk::math::Vector3d, double>> &aSpheres)
{
    initializeLevelsetFieldForSpheres(mKrinoMesh->bulk_data(), mLSFields[0].isovar, aSpheres);
}

void KrinoWrapper::initializeLevelsetFieldForSpheres(const stk::mesh::BulkData &mesh,
                                                     ::krino::FieldRef levelSetField,
                                                     const std::vector<std::pair<stk::math::Vector3d, double>> &spheres)
{
    const ::krino::FieldRef coordsField = mesh.mesh_meta_data().coordinate_field();
    ::krino::Composite_Surface initializationSurfaces("initialization surfaces");
    for (auto &sphere : spheres) initializationSurfaces.add(new ::krino::Sphere(sphere.first, sphere.second));
    compute_nodal_surface_distance(mesh, coordsField, levelSetField, initializationSurfaces);
}

unsigned int KrinoWrapper::getNumTetsInNamedBlock(const std::string &aBlockName) const
{
    unsigned int tTotalSize = 0;
    const stk::mesh::Part *tPart = mKrinoMesh->meta_data().get_part(aBlockName);
    const stk::mesh::Selector tSelector(*tPart);
    const stk::mesh::BucketVector &tBuckets = mKrinoMesh->bulk_data().get_buckets(stk::topology::ELEM_RANK, tSelector);
    for (stk::mesh::BucketVector::const_iterator b_itr = tBuckets.begin(); b_itr != tBuckets.end(); ++b_itr)
    {
        const stk::mesh::Bucket &b = **b_itr;
        tTotalSize += b.size();
    }
    return tTotalSize;
}

void KrinoWrapper::initializePlaneLevelset(const double aNormalX,
                                           const double aNormalY,
                                           const double aNormalZ,
                                           const double aOffset)
{
    initializeLevelsetFieldForPlane(mKrinoMesh->bulk_data(), mLSFields[0].isovar, {aNormalX, aNormalY, aNormalZ},
                                    aOffset);
}

void KrinoWrapper::initializeLevelsetFieldForPlane(const stk::mesh::BulkData &mesh,
                                                   ::krino::FieldRef levelSetField,
                                                   const stk::math::Vector3d &normal,
                                                   const double offset)
{
    const ::krino::FieldRef coordsField = mesh.mesh_meta_data().coordinate_field();
    ::krino::Composite_Surface initializationSurfaces("initialization surfaces");
    initializationSurfaces.add(new ::krino::Plane(normal.data(), offset, 1.0));
    compute_nodal_surface_distance(mesh, coordsField, levelSetField, initializationSurfaces);
}

void KrinoWrapper::resetMesh() { ::krino::CDMesh::reset_mesh_to_original_undecomposed_state(mKrinoMesh->bulk_data()); }

void KrinoWrapper::redistance()
{
    ::krino::Surface_Manager &tSurfaceManager = ::krino::Surface_Manager::get(mKrinoMesh->meta_data());
    const std::vector<std::unique_ptr<::krino::LevelSet>> &tLevelsets = tSurfaceManager.get_levelsets();
    for (auto &tCurLevelset : tLevelsets)
    {
        if (tCurLevelset->name() == kLevelsetName || tCurLevelset->get_composite_name() == kLevelsetName)
        {
            tCurLevelset->redistance();
            break;
        }
    }
}

std::unordered_map<unsigned int, stk::math::Vector3d> KrinoWrapper::getCoordinateValues() const
{
    auto tCurCoordinateValues = std::unordered_map<unsigned int, stk::math::Vector3d>{};
    const unsigned tNumDimensions = mKrinoMesh->meta_data().spatial_dimension();
    const stk::mesh::BucketVector &tNodeBuckets =
        mKrinoMesh->bulk_data().get_buckets(stk::topology::NODE_RANK, stk::mesh::selectField(mLSFields[0].isovar));
    const ::krino::FieldRef tCoordsField = mKrinoMesh->meta_data().coordinate_field();
    for (auto &tCurBucketPtr : tNodeBuckets)
    {
        const stk::mesh::Bucket &tCurBucket = *tCurBucketPtr;
        const double *tBucketCoords = ::krino::field_data<double>(tCoordsField, tCurBucket);
        unsigned int tCntr = 0;
        for (stk::mesh::Entity tCurNodeEntity : tCurBucket)
        {
            const unsigned int tGlobalNodeID = mKrinoMesh->bulk_data().identifier(tCurNodeEntity);
            const stk::math::Vector3d tNodeCoords(&tBucketCoords[static_cast<size_t>(tNumDimensions * tCntr)],
                                                  tNumDimensions);
            tCurCoordinateValues[tGlobalNodeID] = tNodeCoords;
            tCntr++;
        }
    }
    return tCurCoordinateValues;
}

auto KrinoWrapper::getSensitivities() const -> const std::unordered_map<stk::mesh::EntityId, InterfaceNodeDXDP> &
{
    return mSensitivities;
}

auto KrinoWrapper::bulkData() const -> const stk::mesh::BulkData & { return mKrinoMesh->bulk_data(); }

auto KrinoWrapper::getUncutBackgroundMeshSize() const -> unsigned int { return mUncutBackgroundMeshSize; }

}  // namespace plato::third_party_integration::krino
