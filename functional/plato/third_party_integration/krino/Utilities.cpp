#include "plato/third_party_integration/krino/Utilities.hpp"

#include <Akri_AuxMetaData.hpp>     // AuxMetaData::get
#include <Akri_CDFEM_Support.hpp>   //CDFEM_Support
#include <Akri_CDMesh.hpp>          //sierraTimer
#include <Akri_DiagWriter.hpp>      //initialize environment
#include <Akri_LevelSet.hpp>        //LevelSet
#include <Akri_LevelSetPolicy.hpp>  //LSPerInterfacePolicy
#include <Akri_LevelSetShapeSensitivities.hpp>
#include <Akri_MeshHelpers.hpp>           //activate_all_entities
#include <Akri_NodalSurfaceDistance.hpp>  //compute_nodal_surface_distance
#include <Akri_OrientedSideNodes.hpp>
#include <Akri_OutputUtils.hpp>
#include <Akri_TriangleWithSensitivities.hpp>
#include <iterator>
#include <stk_io/StkMeshIoBroker.hpp>  //get_selected_entities
#include <stk_math/StkVector.hpp>
#include <stk_mesh/base/Entity.hpp>
#include <stk_mesh/base/Selector.hpp>
#include <stk_mesh/base/Types.hpp>
#include <stk_util/diag/WriterRegistry.hpp>
#include <stk_util/environment/EnvData.hpp>
#include <stk_util/environment/OutputLog.hpp>
#include <stk_util/parallel/OutputStreams.hpp>
#include <string_view>

#include "plato/third_party_integration/krino/SnappingParameters.hpp"
#include "plato/utilities/ReduceUtilities.hpp"
#include "plato/utilities/TransformIf.hpp"
#include "plato/utilities/Zip.hpp"

namespace plato::third_party_integration::krino
{
namespace
{
constexpr auto kSortByGlobalId = true;
constexpr auto kOutputDescription = std::string_view{"out>null dout>null pout>null"};
constexpr auto kDecompositionMethod = std::string_view{"rib"};
constexpr auto kLevelSetName = std::string_view{"LEVEL_SET"};
constexpr auto kNumberOfLevelSets = 1U;

[[nodiscard]] bool include_void_region_part(const VoidPhase aVoidPhase, const stk::mesh::Part* aPart)
{
    return (aVoidPhase == VoidPhase::kIncludeInMesh || aPart->name().find("_void") == std::string::npos);
}

[[nodiscard]] auto output_selector(const stk::mesh::MetaData& aMeta,
                                   const stk::mesh::Part& aActivePart,
                                   const VoidPhase aVoidPhase) -> stk::mesh::Selector
{
    const auto tOutputCondition = [aVoidPhase](const auto aPartPtr) -> bool
    { return stk::io::is_part_io_part(*aPartPtr) && include_void_region_part(aVoidPhase, aPartPtr); };

    stk::mesh::PartVector tOutputParts;
    utilities::transform_if(
        aMeta.get_parts(), std::back_inserter(tOutputParts), [](const auto aPartPtr) { return aPartPtr; },
        tOutputCondition);

    return aActivePart & stk::mesh::selectUnion(tOutputParts);
}

void setup_fields_for_conforming_decomposition(const stk::mesh::MetaData& aMeta)
{
    ::krino::CDFEM_Support& tCdfemSupport = ::krino::CDFEM_Support::get(aMeta);
    const ::krino::FieldRef tCoordsField = aMeta.coordinate_field();

    tCdfemSupport.set_coords_field(tCoordsField);
    tCdfemSupport.add_edge_interpolation_field(tCoordsField);
    tCdfemSupport.register_parent_node_ids_field();
    tCdfemSupport.setup_levelset_field_stash(
        ::krino::get_levelset_fields(::krino::Phase_Support::get_levelset_fields(aMeta)));
    tCdfemSupport.register_cdfem_snap_displacements_field();
    tCdfemSupport.finalize_fields();
}

[[nodiscard]] auto get_ids_from_entities(const std::vector<stk::mesh::Entity>& aEntities,
                                         const stk::mesh::BulkData& aBulkData) -> std::vector<stk::mesh::EntityId>
{
    std::vector<stk::mesh::EntityId> tIds;
    tIds.reserve(aEntities.size());
    std::transform(aEntities.begin(), aEntities.end(), std::back_inserter(tIds),
                   [&aBulkData](const auto aEntity) { return aBulkData.entity_key(aEntity).id(); });

    std::sort(tIds.begin(), tIds.end());

    return tIds;
}

}  // namespace

void initialize_environment_for_krino(const std::filesystem::path& aLogFile, const MPI_Comm& aComm)
{
    // Initialize STK environment
    stk::EnvData::instance().m_parallelComm = aComm;
    MPI_Comm_size(stk::EnvData::parallel_comm(), &stk::EnvData::instance().m_parallelSize);
    MPI_Comm_rank(stk::EnvData::parallel_comm(), &stk::EnvData::instance().m_parallelRank);

    // Initialize krino logging
    sierra::Diag::registerWriter(std::string{aLogFile}, ::krinolog, ::krino::theDiagWriterParser());
    stk::bind_output_streams(std::string{kOutputDescription});
}

auto retrieve_mpi_communicator_from_krino() -> boost::mpi::communicator
{
    return boost::mpi::communicator(stk::EnvData::instance().m_parallelComm, boost::mpi::comm_duplicate);
}

auto read_and_setup_for_decomposition(const std::filesystem::path& aFilename) -> std::unique_ptr<::krino::MeshInterface>
{
    std::unique_ptr<::krino::MeshFromFile> tMeshFromFile = std::make_unique<::krino::MeshFromFile>(
        aFilename.string(), stk::EnvData::parallel_comm(), std::string{kDecompositionMethod});

    ::krino::LSPerInterfacePolicy::setup_levelsets_on_all_blocks_with_void_phase_for_any_negative_levelset(
        tMeshFromFile->meta_data(), kNumberOfLevelSets);
    ::krino::LevelSet& tLevelSet =
        ::krino::LevelSet::build(tMeshFromFile->meta_data(), std::string{kLevelSetName}, sierra::Diag::sierraTimer());
    tLevelSet.set_levelset_field_name(std::string{kLevelSetName});
    tLevelSet.setup();
    setup_fields_for_conforming_decomposition(tMeshFromFile->meta_data());
    tMeshFromFile->populate_mesh();
    ::krino::activate_all_entities(tMeshFromFile->bulk_data(),
                                   ::krino::AuxMetaData::get(tMeshFromFile->meta_data()).active_part());
    return tMeshFromFile;
}

auto make_level_set_field_from_primitives(const LevelSetPrimitives& aLevelSetPrimitives,
                                          const stk::mesh::BulkData& aBulkData) -> std::vector<::krino::LS_Field>
{
    auto tInitializationSurfaces = make_krino_composite_surface();
    append_spheres(tInitializationSurfaces, aLevelSetPrimitives.mSpheres);
    append_planes(tInitializationSurfaces, aLevelSetPrimitives.mPlanes);

    std::vector<::krino::LS_Field> tField = ::krino::Phase_Support::get_levelset_fields(aBulkData.mesh_meta_data());
    ::krino::compute_nodal_surface_distance(aBulkData, aBulkData.mesh_meta_data().coordinate_field(),
                                            tField.front().isovar, tInitializationSurfaces);
    return tField;
}

auto make_level_set_field_from_fixed_value(::krino::MeshInterface& aKrinoMesh, const double aFixedLevelSetValue)
    -> std::vector<::krino::LS_Field>
{
    std::vector<::krino::LS_Field> tField = ::krino::Phase_Support::get_levelset_fields(aKrinoMesh.meta_data());
    const auto tNodes = node_entities_in_mesh(aKrinoMesh, tField);
    for (const auto tNode : tNodes)
    {
        level_set_value(tField, tNode) = aFixedLevelSetValue;
    }
    return tField;
}

auto background_node_ids(const ::krino::MeshInterface& aKrinoMesh,
                         const std::vector<::krino::LS_Field>& aLevelSetFields) -> std::vector<stk::mesh::EntityId>
{
    const auto tBackgroundNodes = node_entities_in_mesh(aKrinoMesh, aLevelSetFields);
    return utilities::unique_vector_gather(get_ids_from_entities(tBackgroundNodes, aKrinoMesh.bulk_data()),
                                           retrieve_mpi_communicator_from_krino());
}

namespace
{
[[nodiscard]] auto get_cut_mesh_node_entities(const ::krino::MeshInterface& aKrinoMesh, const VoidPhase aVoidPhase)
    -> stk::mesh::EntityVector
{
    const auto tSelector = output_selector(aKrinoMesh.meta_data(),
                                           ::krino::AuxMetaData::get(aKrinoMesh.meta_data()).active_part(), aVoidPhase);

    stk::mesh::EntityVector tNodes;
    stk::mesh::get_selected_entities(tSelector, aKrinoMesh.bulk_data().buckets(stk::topology::NODE_RANK), tNodes,
                                     kSortByGlobalId);
    return tNodes;
}

}  // namespace

auto cut_mesh_node_ids(const ::krino::MeshInterface& aKrinoMesh, const VoidPhase aVoidPhase)
    -> std::vector<stk::mesh::EntityId>
{
    const auto tNodes = get_cut_mesh_node_entities(aKrinoMesh, aVoidPhase);
    return utilities::unique_vector_gather(get_ids_from_entities(tNodes, aKrinoMesh.bulk_data()),
                                           retrieve_mpi_communicator_from_krino());
}

void cut_mesh(stk::mesh::BulkData& aBulkData,
              const std::vector<::krino::LS_Field>& aLevelSetFields,
              const SnappingParameters aSnappingParameters)
{
    const auto& tMeta = aBulkData.mesh_meta_data();
    auto& tAuxMeta = ::krino::AuxMetaData::get(tMeta);
    auto& tCdfemSupport = ::krino::CDFEM_Support::get(tMeta);
    tCdfemSupport.set_cdfem_edge_degeneracy_handling(
        ::krino::Edge_Degeneracy_Handling::SNAP_TO_INTERFACE_WHEN_QUALITY_ALLOWS_THEN_SNAP_TO_NODE);
    tCdfemSupport.set_snapping_sharp_feature_angle_in_degrees(aSnappingParameters.mSharpFeatureAngle);
    tCdfemSupport.set_max_edge_snap(aSnappingParameters.mMaxSnappingEdgeLength);
    auto& tPhaseSupport = ::krino::Phase_Support::get(tMeta);
    auto tInterfaceGeometry =
        ::krino::create_levelset_geometry(static_cast<int>(tMeta.spatial_dimension()), tAuxMeta.active_part(),
                                          tCdfemSupport, tPhaseSupport, aLevelSetFields);
    tAuxMeta.clear_force_64bit_flag();
    ::krino::CDMesh::decompose_mesh(aBulkData, *tInterfaceGeometry);
}

auto get_level_set_field_name() -> std::string { return std::string{kLevelSetName}; }

auto get_level_set_values(const ::krino::MeshInterface& aKrinoMesh,
                          const std::vector<::krino::LS_Field>& aLevelSetFields)
    -> std::unordered_map<stk::mesh::EntityId, double>
{
    const auto tNodes = node_entities_in_mesh(aKrinoMesh, aLevelSetFields);
    std::unordered_map<stk::mesh::EntityId, double> tLevelSetValues;
    tLevelSetValues.reserve(tNodes.size());

    std::transform(tNodes.begin(), tNodes.end(), std::inserter(tLevelSetValues, tLevelSetValues.begin()),
                   [&aKrinoMesh, &aLevelSetFields](const auto& aNode)
                   {
                       const auto tId = aKrinoMesh.bulk_data().entity_key(aNode).id();
                       return std::make_pair(tId, level_set_value(aLevelSetFields, aNode));
                   });

    return utilities::reduce_map(tLevelSetValues, retrieve_mpi_communicator_from_krino());
}

auto node_entities_in_mesh(const ::krino::MeshInterface& aKrinoMesh,
                           const std::vector<::krino::LS_Field>& aLevelSetFields) -> stk::mesh::EntityVector
{
    ::krino::CDFEM_Support& cdfemSupport = ::krino::CDFEM_Support::get(aKrinoMesh.meta_data());
    const stk::mesh::Selector tSelector =
        stk::mesh::selectField(aLevelSetFields.front().isovar) & !cdfemSupport.get_child_node_part();
    stk::mesh::EntityVector tNodes;
    stk::mesh::get_selected_entities(tSelector, aKrinoMesh.bulk_data().buckets(stk::topology::NODE_RANK), tNodes,
                                     kSortByGlobalId);
    return tNodes;
}

void write_mesh(const stk::mesh::BulkData& aBulkData,
                const std::filesystem::path& aOutputFileName,
                const VoidPhase aVoidPhase)
{
    const auto tOutputSelector = output_selector(
        aBulkData.mesh_meta_data(), ::krino::AuxMetaData::get(aBulkData.mesh_meta_data()).active_part(), aVoidPhase);
    constexpr auto tStepIndex = int{1};
    constexpr auto tTime = double{0.0};
    ::krino::output_composed_mesh_with_fields(aBulkData, tOutputSelector, aOutputFileName.string(), tStepIndex, tTime);
}

TriangleAreaSensitivity get_d_area_d_nodal_coords_from_tri(const stk_io::Triangle& aTriangle)
{
    constexpr size_t tNumDimensions{3};
    constexpr size_t tNumNodesPerTriangle{3};
    TriangleAreaSensitivity tTriangleAreaSensitivity;
    std::vector<double> tDAreaDCoords = get_d_area_d_nodal_coords_from_tri_coords(
        {aTriangle.p0.x, aTriangle.p0.y, aTriangle.p0.z, aTriangle.p1.x, aTriangle.p1.y, aTriangle.p1.z, aTriangle.p2.x,
         aTriangle.p2.y, aTriangle.p2.z});
    for (size_t tNodeIndex = 0; tNodeIndex < tNumNodesPerTriangle; tNodeIndex++)
    {
        for (size_t tDimIndex = 0; tDimIndex < tNumDimensions; ++tDimIndex)
        {
            tTriangleAreaSensitivity[aTriangle.global_ids[tNodeIndex]][tDimIndex] =
                tDAreaDCoords[tNumDimensions * tNodeIndex + tDimIndex];
        }
    }
    return tTriangleAreaSensitivity;
}

TriangleNormalSensitivity get_d_normal_d_nodal_coords_from_tri(const stk_io::Triangle& aTriangle)
{
    constexpr size_t tNumDimensions{3};
    constexpr size_t tNumNodesPerTriangle{3};
    constexpr size_t tNumNodeSensitivities = tNumDimensions * tNumDimensions;
    TriangleNormalSensitivity tTriangleNormalSensitivity;
    std::vector<double> tDNormalDCoords = get_d_normal_d_nodal_coords_from_tri_coords(
        {aTriangle.p0.x, aTriangle.p0.y, aTriangle.p0.z, aTriangle.p1.x, aTriangle.p1.y, aTriangle.p1.z, aTriangle.p2.x,
         aTriangle.p2.y, aTriangle.p2.z});
    for (size_t tNodeIndex = 0; tNodeIndex < tNumNodesPerTriangle; tNodeIndex++)
    {
        const size_t tNodeStride = tNodeIndex * tNumNodeSensitivities;
        for (size_t tSpatialDimIndex = 0; tSpatialDimIndex < tNumDimensions; ++tSpatialDimIndex)
        {
            const size_t tSpatialStride = tSpatialDimIndex * tNumDimensions;
            for (size_t tNormalDimIndex = 0; tNormalDimIndex < tNumDimensions; ++tNormalDimIndex)
            {
                tTriangleNormalSensitivity[aTriangle.global_ids[tNodeIndex]][tSpatialDimIndex][tNormalDimIndex] =
                    tDNormalDCoords[tNodeStride + tSpatialStride + tNormalDimIndex];
            }
        }
    }
    return tTriangleNormalSensitivity;
}

double get_tri_area_from_nodal_coords(const std::vector<double>& aNodalCoords)
{
    const stk::math::Vector3d tPoint0{aNodalCoords[0], aNodalCoords[1], aNodalCoords[2]};
    const stk::math::Vector3d tPoint1{aNodalCoords[3], aNodalCoords[4], aNodalCoords[5]};
    const stk::math::Vector3d tPoint2{aNodalCoords[6], aNodalCoords[7], aNodalCoords[8]};
    return ::krino::TriangleWithSens::area_and_optional_sensitivities(tPoint0, tPoint1, tPoint2, nullptr);
}

std::vector<double> get_tri_normal_from_nodal_coords(const std::vector<double>& aNodalCoords)
{
    const stk::math::Vector3d tPoint0{aNodalCoords[0], aNodalCoords[1], aNodalCoords[2]};
    const stk::math::Vector3d tPoint1{aNodalCoords[3], aNodalCoords[4], aNodalCoords[5]};
    const stk::math::Vector3d tPoint2{aNodalCoords[6], aNodalCoords[7], aNodalCoords[8]};
    stk::math::Vector3d tNormal =
        ::krino::TriangleWithSens::normal_and_optional_sensitivities(tPoint0, tPoint1, tPoint2, nullptr);
    return std::vector<double>{tNormal[0], tNormal[1], tNormal[2]};
}

std::vector<double> get_d_area_d_nodal_coords_from_tri_coords(const std::vector<double>& aNodalCoords)
{
    constexpr auto tNumberSpatialDimensions{3};
    constexpr auto tNumberNodesPerTriangle{3};
    constexpr auto tNumSensitivities = tNumberSpatialDimensions * tNumberNodesPerTriangle;

    // Get the change in triangle area and normal with changes in triangle nodal coordinates (3 triplets)
    std::vector<double> tDAreaDNodalCoordinates(tNumSensitivities);
    const stk::math::Vector3d tPoint0{aNodalCoords[0], aNodalCoords[1], aNodalCoords[2]};
    const stk::math::Vector3d tPoint1{aNodalCoords[3], aNodalCoords[4], aNodalCoords[5]};
    const stk::math::Vector3d tPoint2{aNodalCoords[6], aNodalCoords[7], aNodalCoords[8]};
    ::krino::TriangleWithSens::area_and_optional_sensitivities(tPoint0, tPoint1, tPoint2,
                                                               tDAreaDNodalCoordinates.data());
    return tDAreaDNodalCoordinates;
}

std::vector<double> get_d_normal_d_nodal_coords_from_tri_coords(const std::vector<double>& aNodalCoords)
{
    constexpr auto tNumberSpatialDimensions{3};
    constexpr auto tNumberNodesPerTriangle{3};
    constexpr auto tNumSensitivities = tNumberSpatialDimensions * tNumberNodesPerTriangle * tNumberSpatialDimensions;

    // Get the change in triangle area and normal with changes in triangle nodal coordinates (3 triplets)
    std::vector<double> tDNormalDNodalCoordinates(tNumSensitivities);
    const stk::math::Vector3d tPoint0{aNodalCoords[0], aNodalCoords[1], aNodalCoords[2]};
    const stk::math::Vector3d tPoint1{aNodalCoords[3], aNodalCoords[4], aNodalCoords[5]};
    const stk::math::Vector3d tPoint2{aNodalCoords[6], aNodalCoords[7], aNodalCoords[8]};
    ::krino::TriangleWithSens::normal_and_optional_sensitivities(tPoint0, tPoint1, tPoint2,
                                                                 tDNormalDNodalCoordinates.data());
    return tDNormalDNodalCoordinates;
}

std::vector<stk::mesh::Entity> get_owned_interface_sides(const stk::mesh::BulkData& aBulkData,
                                                         const stk::mesh::Selector& aInterfaceSelector)
{
    std::vector<stk::mesh::Entity> tInterfaceSides;

    for (auto* tBucket : aBulkData.get_buckets(aBulkData.mesh_meta_data().side_rank(),
                                               aBulkData.mesh_meta_data().locally_owned_part() & aInterfaceSelector))
    {
        tInterfaceSides.insert(tInterfaceSides.end(), tBucket->begin(), tBucket->end());
    }

    return tInterfaceSides;
}

auto get_interface_triangles(const stk::mesh::BulkData& aBulkData,
                             const std::string& aSidesetName,
                             const PartReferenceVector& aDesignDomainBlocks) -> std::vector<stk_io::Triangle>
{
    std::vector<stk_io::Triangle> tTriList;

    const stk::mesh::Selector tTriSelector(*(aBulkData.mesh_meta_data().get_part(aSidesetName)));
    std::vector<const stk::mesh::Part*> tParts;
    for (const auto& tCurBlock : aDesignDomainBlocks)
    {
        std::cout << "Block ID: " << tCurBlock.get().id() << "; Name: " << tCurBlock.get().name() << std::endl;
        if (tCurBlock.get().name().find("void") == std::string::npos)
        {
            tParts.push_back(&(tCurBlock.get()));
        }
    }
    const stk::mesh::Selector tTetSelector{stk::mesh::selectUnion(tParts)};
    const std::vector<stk::mesh::Entity> tInterfaceSides = get_owned_interface_sides(aBulkData, tTriSelector);
    const stk::mesh::FieldBase* const tCoordsField = aBulkData.mesh_meta_data().coordinate_field();
    std::vector<stk_io::Triangle> tTriangles(tInterfaceSides.size());
    size_t tNumTris = 0;
    for (const auto& tInterfaceSide : tInterfaceSides)
    {
        const std::array<stk::mesh::Entity, 3> tSideNodes =
            ::krino::get_oriented_triangle_side_nodes(aBulkData, tTetSelector, tInterfaceSide);
        std::vector<common::Coordinate> tTriCoords(3);
        for (size_t i = 0; i < 3; ++i)
        {
            const double* tCoords = static_cast<const double*>(stk::mesh::field_data(*tCoordsField, tSideNodes[i]));
            tTriCoords[i].x = tCoords[0];
            tTriCoords[i].y = tCoords[1];
            tTriCoords[i].z = tCoords[2];
        }
        tTriangles[tNumTris++] = stk_io::Triangle{
            tTriCoords[0],
            tTriCoords[1],
            tTriCoords[2],
            {(unsigned int)aBulkData.identifier(tSideNodes[0]), (unsigned int)aBulkData.identifier(tSideNodes[1]),
             (unsigned int)aBulkData.identifier(tSideNodes[2])}};
    }
    return tTriangles;
}
}  // namespace plato::third_party_integration::krino
