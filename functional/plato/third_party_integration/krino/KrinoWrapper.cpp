#include "plato/third_party_integration/krino/KrinoWrapper.hpp"

#include <Akri_AnalyticSurf.hpp>
#include <Akri_AuxMetaData.hpp>
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
#include <numeric>
#include <stk_io/StkMeshIoBroker.hpp>
#include <stk_mesh/base/MetaData.hpp>
#include <stk_mesh/base/Types.hpp>
#include <stk_util/environment/EnvData.hpp>
#include <string_view>

#include "plato/analysis/AnalysisDomainMesh.hpp"
#include "plato/analysis/AnalysisDomainMeshSequentialView.hpp"
#include "plato/third_party_integration/krino/Utilities.hpp"
#include "plato/utilities/Enumerate.hpp"
#include "plato/utilities/MultiVectorView.hpp"
#include "plato/utilities/Zip.hpp"

namespace plato::third_party_integration::krino
{
namespace
{
constexpr auto kLevelSetName = std::string_view{"LEVEL_SET"};
constexpr auto kDecompositionMethod = std::string_view{"rib"};
constexpr auto kInitializationSurfaces = std::string_view{"initialization surfaces"};

[[nodiscard]] auto node_ids_for_nodes(const stk::mesh::BulkData &aMesh,
                                      const std::vector<stk::mesh::Entity> &aParentNodes)
    -> std::vector<stk::mesh::EntityId>
{
    auto tParentNodeIds = std::vector<stk::mesh::EntityId>{};
    tParentNodeIds.reserve(aParentNodes.size());
    std::transform(aParentNodes.cbegin(), aParentNodes.cend(), std::back_inserter(tParentNodeIds),
                   [&aMesh](const auto &tParentNode) { return aMesh.identifier(tParentNode); });
    return tParentNodeIds;
}

[[nodiscard]] bool include_void_region_part(const VoidPhase aVoidRegion, const stk::mesh::Part *aPart)
{
    return (aVoidRegion == VoidPhase::kIncludeInMesh || aPart->name().find("_void") == std::string::npos);
}

[[nodiscard]] auto coords_level_sets(const ::krino::FieldRef aCoordsField,
                                     const ::krino::FieldRef aLevelSetField,
                                     const std::vector<stk::mesh::Entity> &aParentNodes)
    -> std::vector<stk::math::Vector3d>
{
    assert(2 == aParentNodes.size());

    const auto tX0 = stk::math::Vector3d(::krino::field_data<double>(aCoordsField, aParentNodes[0]));
    const auto tX1 = stk::math::Vector3d(::krino::field_data<double>(aCoordsField, aParentNodes[1]));
    const auto tDx = tX1 - tX0;

    const auto tLs0 = *::krino::field_data<double>(aLevelSetField, aParentNodes[0]);
    const auto tLs1 = *::krino::field_data<double>(aLevelSetField, aParentNodes[1]);
    const auto tSqrLo = (tLs0 - tLs1) * (tLs0 - tLs1);

    return std::vector<stk::math::Vector3d>{(-tLs1 / tSqrLo) * tDx, (tLs0 / tSqrLo) * tDx};
}

void cut_mesh(stk::mesh::BulkData &aMesh, const std::vector<::krino::LS_Field> &aLevelSetFields)
{
    auto &tMeta = aMesh.mesh_meta_data();
    auto &tAuxMeta = ::krino::AuxMetaData::get(tMeta);
    auto &tCdfemSupport = ::krino::CDFEM_Support::get(tMeta);
    auto &tPhaseSupport = ::krino::Phase_Support::get(tMeta);
    auto tInterfaceGeometry =
        ::krino::create_levelset_geometry(static_cast<int>(tMeta.spatial_dimension()), tAuxMeta.active_part(),
                                          tCdfemSupport, tPhaseSupport, aLevelSetFields);
    tAuxMeta.clear_force_64bit_flag();
    ::krino::CDMesh::decompose_mesh(aMesh, *tInterfaceGeometry);
}

[[nodiscard]] auto output_selector(const stk::mesh::MetaData &aMeta,
                                   const stk::mesh::Part &aActivePart,
                                   const VoidPhase aVoidRegion) -> stk::mesh::Selector
{
    stk::mesh::PartVector tOutputParts;
    for (auto *aPart : aMeta.get_parts())
    {
        if (stk::io::is_part_io_part(*aPart) && include_void_region_part(aVoidRegion, aPart))
        {
            tOutputParts.push_back(aPart);
        }
    }
    return aActivePart & stk::mesh::selectUnion(tOutputParts);
}

[[nodiscard]] auto node_entities_in_mesh(const ::krino::MeshInterface &aKrinoMesh,
                                         const std::vector<::krino::LS_Field> &aLevelSetFields)
    -> stk::mesh::EntityVector
{
    ::krino::CDFEM_Support &cdfemSupport = ::krino::CDFEM_Support::get(aKrinoMesh.meta_data());
    const stk::mesh::Selector tSelector =
        stk::mesh::selectField(aLevelSetFields.front().isovar) & !cdfemSupport.get_child_node_part();
    stk::mesh::EntityVector tNodes;
    stk::mesh::get_selected_entities(tSelector, aKrinoMesh.bulk_data().buckets(stk::topology::NODE_RANK), tNodes);
    return tNodes;
}

void append_spheres(::krino::Composite_Surface &aSurfaces, const std::vector<Sphere> &aSpheres)
{
    for (const auto &tSphere : aSpheres)
    {
        aSurfaces.add(new ::krino::Sphere({tSphere.mCenter.x, tSphere.mCenter.y, tSphere.mCenter.z}, tSphere.mRadius));
    }
}
void append_planes(::krino::Composite_Surface &aSurfaces, const std::vector<Plane> &aPlanes)
{
    for (const auto &tPlane : aPlanes)
    {
        const auto tNormal = std::array{tPlane.mNormal.x, tPlane.mNormal.y, tPlane.mNormal.z};
        aSurfaces.add(new ::krino::Plane(tNormal.data(), tPlane.mOffset, 1.0));
    }
}

void initialize_level_set_field_to_fixed_value(::krino::MeshInterface &aKrinoMesh,
                                               std::vector<::krino::LS_Field> &aLevelSetFields,
                                               const double aFixedLevelSetValue)
{
    const auto tNodes = node_entities_in_mesh(aKrinoMesh, aLevelSetFields);
    for (const auto tNode : tNodes)
    {
        *::krino::field_data<double>(aLevelSetFields.front().isovar, tNode) = aFixedLevelSetValue;
    }
}

void initialize_level_sets_from_primitives(const LevelSetPrimitives &aLevelSetPrimitives,
                                           const ::krino::MeshInterface &aKrinoMesh,
                                           std::vector<::krino::LS_Field> &aLevelSetFields)
{
    const auto &tMesh = aKrinoMesh.bulk_data();
    auto tInitializationSurfaces = ::krino::Composite_Surface{std::string{kInitializationSurfaces}};
    append_spheres(tInitializationSurfaces, aLevelSetPrimitives.mSpheres);
    append_planes(tInitializationSurfaces, aLevelSetPrimitives.mPlanes);
    ::krino::compute_nodal_surface_distance(tMesh, tMesh.mesh_meta_data().coordinate_field(),
                                            aLevelSetFields.front().isovar, tInitializationSurfaces);
}

void setup_fields_for_conforming_decomposition(const stk::mesh::MetaData &aMeta)
{
    ::krino::CDFEM_Support &tCdfemSupport = ::krino::CDFEM_Support::get(aMeta);
    const ::krino::FieldRef tCoordsField = aMeta.coordinate_field();

    tCdfemSupport.set_coords_field(tCoordsField);
    tCdfemSupport.add_edge_interpolation_field(tCoordsField);
    tCdfemSupport.register_parent_node_ids_field();
}

auto read_and_setup_for_decomposition(const std::filesystem::path &aFilename) -> std::unique_ptr<::krino::MeshInterface>
{
    std::unique_ptr<::krino::MeshFromFile> tMeshFromFile = std::make_unique<::krino::MeshFromFile>(
        aFilename.string(), stk::EnvData::parallel_comm(), std::string{kDecompositionMethod});
    constexpr auto tNumberOfLevelSets = 1U;
    ::krino::LSPerInterfacePolicy::setup_levelsets_on_all_blocks_with_void_phase_for_any_negative_levelset(
        tMeshFromFile->meta_data(), tNumberOfLevelSets);
    ::krino::LevelSet &tLevelSet =
        ::krino::LevelSet::build(tMeshFromFile->meta_data(), std::string{kLevelSetName}, sierra::Diag::sierraTimer());
    tLevelSet.set_distance_name(std::string{kLevelSetName});
    tLevelSet.setup();
    setup_fields_for_conforming_decomposition(tMeshFromFile->meta_data());
    tMeshFromFile->populate_mesh();
    ::krino::activate_all_entities(tMeshFromFile->bulk_data(),
                                   ::krino::AuxMetaData::get(tMeshFromFile->meta_data()).active_part());
    return tMeshFromFile;
}

void set_level_set_fields(::krino::MeshInterface &aKrinoMesh,
                          std::vector<::krino::LS_Field> &aLevelSetFields,
                          const analysis::AnalysisDomainMesh &aAnalysisDomainMesh)
{
    for (const auto &tScalarFieldValueProxy : analysis::AnalysisDomainMeshSequentialView{aAnalysisDomainMesh})
    {
        const auto &tScalarFieldValue = static_cast<analysis::ScalarFieldValue>(tScalarFieldValueProxy);
        const auto tStkEntity =
            aKrinoMesh.bulk_data().get_entity(stk::topology::NODE_RANK, tScalarFieldValue.mGlobalMeshEntityID);
        *::krino::field_data<double>(aLevelSetFields.front().isovar, tStkEntity) = tScalarFieldValue.mValue;
    }
    cut_mesh(aKrinoMesh.bulk_data(), aLevelSetFields);
}

}  // namespace

KrinoWrapper::KrinoWrapper(const std::filesystem::path &aFilename,
                           const LevelSetPrimitives &aLevelSetPrimitives,
                           const VoidPhase aVoidRegion)
    : mVoidRegion(aVoidRegion),
      mKrinoMesh(read_and_setup_for_decomposition(aFilename)),
      mLevelSetFields(::krino::Phase_Support::get_levelset_fields(mKrinoMesh->meta_data()))
{
    initialize_level_sets_from_primitives(aLevelSetPrimitives, *mKrinoMesh, mLevelSetFields);
    cut_mesh(mKrinoMesh->bulk_data(), mLevelSetFields);
}

KrinoWrapper::KrinoWrapper(const std::filesystem::path &aFilename,
                           const std::vector<double> &aLevelSetValues,
                           const VoidPhase aVoidRegion)
    : mVoidRegion(aVoidRegion),
      mKrinoMesh(read_and_setup_for_decomposition(aFilename)),
      mLevelSetFields(::krino::Phase_Support::get_levelset_fields(mKrinoMesh->meta_data()))
{
    setLevelSetValues(aLevelSetValues);
}

KrinoWrapper::KrinoWrapper(const analysis::AnalysisDomainMesh &aAnalysisDomainMesh,
                           const double aFixedLevelSetValue,
                           const VoidPhase aVoidRegion)
    : mVoidRegion(aVoidRegion),
      mKrinoMesh(read_and_setup_for_decomposition(aAnalysisDomainMesh.mFileName)),
      mLevelSetFields(::krino::Phase_Support::get_levelset_fields(mKrinoMesh->meta_data()))
{
    initialize_level_set_field_to_fixed_value(*mKrinoMesh, mLevelSetFields, aFixedLevelSetValue);
    set_level_set_fields(*mKrinoMesh, mLevelSetFields, aAnalysisDomainMesh);
}

void KrinoWrapper::setLevelSetValues(const std::vector<double> &aValuesIn)
{
    ::krino::CDMesh::reset_mesh_to_original_undecomposed_state(mKrinoMesh->bulk_data());
    const auto tNodes = node_entities_in_mesh(*mKrinoMesh, mLevelSetFields);
    assert(aValuesIn.size() == tNodes.size());

    for (const auto &[tNode, tLevelSetValue] : utilities::Zip{tNodes, aValuesIn})
    {
        *::krino::field_data<double>(mLevelSetFields.front().isovar, tNode) = tLevelSetValue;
    }
    cut_mesh(mKrinoMesh->bulk_data(), mLevelSetFields);
}

void KrinoWrapper::writeMesh(const std::filesystem::path &aFilename)
{
    const auto tOutputSelector = output_selector(
        mKrinoMesh->meta_data(), ::krino::AuxMetaData::get(mKrinoMesh->meta_data()).active_part(), mVoidRegion);
    constexpr auto tStepIndex = int{1};
    constexpr auto tTime = double{0.0};
    ::krino::output_composed_mesh_with_fields(mKrinoMesh->bulk_data(), tOutputSelector, aFilename.string(), tStepIndex,
                                              tTime);
}

std::vector<double> KrinoWrapper::levelSetValues() const
{
    const auto tNodes = node_entities_in_mesh(*mKrinoMesh, mLevelSetFields);
    std::vector<double> tReturn;
    tReturn.reserve(tNodes.size());
    std::transform(tNodes.begin(), tNodes.end(), std::back_inserter(tReturn),
                   [this](const auto &aNode)
                   { return *::krino::field_data<double>(mLevelSetFields.front().isovar, aNode); });

    return tReturn;
}

void KrinoWrapper::redistance()
{
    const auto &tSurfaceManager = ::krino::Surface_Manager::get(mKrinoMesh->meta_data());
    const auto &tLevelSets = tSurfaceManager.get_levelsets();

    const auto tMatchingLevelSet = std::find_if(
        tLevelSets.cbegin(), tLevelSets.cend(),
        [](const auto &tLevelSet)
        { return tLevelSet->name() == kLevelSetName || tLevelSet->get_composite_name() == kLevelSetName; });

    if (tMatchingLevelSet != tLevelSets.cend())
    {
        (*tMatchingLevelSet)->redistance();
    }
}

auto KrinoWrapper::coordinates() const -> std::unordered_map<unsigned int, stk::math::Vector3d>
{
    auto tCurCoordinateValues = std::unordered_map<unsigned int, stk::math::Vector3d>{};
    const unsigned tNumDimensions = mKrinoMesh->meta_data().spatial_dimension();
    const stk::mesh::BucketVector &tNodeBuckets = mKrinoMesh->bulk_data().get_buckets(
        stk::topology::NODE_RANK, stk::mesh::selectField(mLevelSetFields.front().isovar));
    const ::krino::FieldRef tCoordsField = mKrinoMesh->meta_data().coordinate_field();
    for (auto &tCurBucketPtr : tNodeBuckets)
    {
        const stk::mesh::Bucket &tCurBucket = *tCurBucketPtr;
        const double *tBucketCoords = ::krino::field_data<double>(tCoordsField, tCurBucket);
        for (const auto &[tVectorIndex, tCurNodeEntity] : utilities::enumerate(tCurBucket))
        {
            const unsigned int tGlobalNodeID = mKrinoMesh->bulk_data().identifier(tCurNodeEntity);
            const auto tCoordinateIndex = static_cast<size_t>(tNumDimensions * tVectorIndex);
            tCurCoordinateValues[tGlobalNodeID] = stk::math::Vector3d(&tBucketCoords[tCoordinateIndex], tNumDimensions);
        }
    }
    return tCurCoordinateValues;
}

auto KrinoWrapper::sensitivities() const -> std::unordered_map<stk::mesh::EntityId, LevelSetJacobianColumn>
{
    const stk::mesh::BulkData &tMesh = mKrinoMesh->bulk_data();

    const ::krino::FieldRef coordsField = tMesh.mesh_meta_data().coordinate_field();
    const ::krino::CDFEM_Support &cdfemSupport = ::krino::CDFEM_Support::get(tMesh.mesh_meta_data());
    std::vector<::krino::ChildNodeStencil> tChildNodeStencils;
    ::krino::fill_child_node_stencils(tMesh, cdfemSupport.get_child_node_part(),
                                      cdfemSupport.get_parent_node_ids_field(),
                                      cdfemSupport.get_parent_node_weights_field(), tChildNodeStencils);

    auto tSensitivityMap = std::unordered_map<stk::mesh::EntityId, LevelSetJacobianColumn>{};
    tSensitivityMap.reserve(tChildNodeStencils.size());
    for (auto &tStencil : tChildNodeStencils)
    {
        tSensitivityMap[tMesh.identifier(tStencil.childNode)] = LevelSetJacobianColumn{
            node_ids_for_nodes(tMesh, tStencil.parentNodes),
            coords_level_sets(coordsField, mLevelSetFields.front().isovar, tStencil.parentNodes)};
    }

    return tSensitivityMap;
}

auto KrinoWrapper::bulkData() const -> const stk::mesh::BulkData & { return mKrinoMesh->bulk_data(); }

}  // namespace plato::third_party_integration::krino
