#include "plato/third_party_integration/krino/Utilities.hpp"

#include <Akri_AuxMetaData.hpp>    // AuxMetaData::get
#include <Akri_CDFEM_Support.hpp>  //CDFEM_Support
#include <Akri_CDMesh.hpp>         //sierraTimer
#include <Akri_CreateInterfaceGeometry.hpp>
#include <Akri_LevelSet.hpp>  //LevelSet
#include <Akri_LevelSetShapeSensitivities.hpp>
#include <Akri_MeshHelpers.hpp>  //activate_all_entities
#include <Akri_MeshInterface.hpp>
#include <Akri_NodalSurfaceDistance.hpp>  //compute_nodal_surface_distance
#include <Akri_OutputUtils.hpp>
#include <Akri_Phase_Support.hpp>
#include <algorithm>
#include <boost/mpi/communicator.hpp>
#include <cassert>
#include <filesystem>
#include <iterator>
#include <memory>
#include <stk_io/DatabasePurpose.hpp>
#include <stk_io/StkMeshIoBroker.hpp>  //get_selected_entities
#include <stk_mesh/base/Entity.hpp>
#include <stk_mesh/base/GetEntities.hpp>
#include <stk_mesh/base/MetaData.hpp>
#include <stk_mesh/base/Selector.hpp>
#include <stk_mesh/base/Types.hpp>
#include <stk_topology/topology_decl.hpp>
#include <stk_util/environment/EnvData.hpp>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

#include "plato/third_party_integration/krino/BackgroundMesh.hpp"
#include "plato/third_party_integration/krino/SnappingParameters.hpp"
#include "plato/utilities/ContainerHelpers.hpp"
#include "plato/utilities/ReduceUtilities.hpp"
#include "plato/utilities/TransformIf.hpp"

namespace plato::third_party_integration::krino
{
namespace
{
constexpr auto kSortByGlobalId = true;
constexpr auto kLevelSetName = std::string_view{"LS"};
constexpr auto kVoidPhaseName = std::string_view{"void"};
const auto kLevelSetFields = std::vector<std::string>{"LS", "LS_COPYFORSNAPPING"};

[[nodiscard]] auto include_void_region_part(const VoidPhase aVoidPhase, const stk::mesh::Part* aPart) -> bool
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

[[nodiscard]] auto get_ids_from_entities(const std::vector<stk::mesh::Entity>& aEntities,
                                         const stk::mesh::BulkData& aBulkData) -> std::vector<stk::mesh::EntityId>
{
    auto tIds = utilities::reserved_container<std::vector<stk::mesh::EntityId>>(aEntities.size());
    std::ranges::transform(aEntities, std::back_inserter(tIds),
                           [&aBulkData](const auto aEntity) { return aBulkData.entity_key(aEntity).id(); });

    std::ranges::sort(tIds);

    return tIds;
}

auto use_snapping(const SnappingParameters& aSnappingParameters) -> bool
{
    return aSnappingParameters.mMaxSnappingEdgeLength != 0.0;
}
}  // namespace

void set_stk_communicator_for_krino(const boost::mpi::communicator& aCommunicator)
{
    stk::EnvData::instance().initialize(aCommunicator);
}

auto retrieve_mpi_communicator_from_krino() -> boost::mpi::communicator
{
    return {stk::EnvData::instance().m_parallelComm, boost::mpi::comm_duplicate};
}

auto background_node_ids(const BackgroundMesh& aBackgroundMesh) -> std::vector<stk::mesh::EntityId>
{
    const auto tBackgroundNodes = node_entities_in_mesh(aBackgroundMesh);
    return utilities::unique_vector_gather(
        get_ids_from_entities(tBackgroundNodes, aBackgroundMesh.mKrinoMesh->bulk_data()),
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

void cut_mesh(const BackgroundMesh& aBackgroundMesh, const SnappingParameters aSnappingParameters)
{
    const auto& tMeta = aBackgroundMesh.mKrinoMesh->bulk_data().mesh_meta_data();
    auto& tAuxMeta = ::krino::AuxMetaData::get(tMeta);
    auto& tCdfemSupport = ::krino::CDFEM_Support::get(tMeta);
    auto& tPhaseSupport = ::krino::Phase_Support::get(tMeta);

    if (use_snapping(aSnappingParameters))
    {
        tCdfemSupport.set_cdfem_edge_degeneracy_handling(
            ::krino::Edge_Degeneracy_Handling::SNAP_TO_INTERFACE_WHEN_QUALITY_ALLOWS_THEN_SNAP_TO_NODE);
        tCdfemSupport.set_snapping_sharp_feature_angle_in_degrees(aSnappingParameters.mSharpFeatureAngle);
        tCdfemSupport.set_max_edge_snap(aSnappingParameters.mMaxSnappingEdgeLength);
    }

    const auto& tLevelSetFields =
        use_snapping(aSnappingParameters)
            ? ::krino::update_levelset_copies_to_prepare_for_snapping(tMeta, aBackgroundMesh.mLevelSetFields)
            : aBackgroundMesh.mLevelSetFields;

    const auto tInterfaceGeometry =
        ::krino::create_levelset_geometry(static_cast<int>(tMeta.spatial_dimension()), tAuxMeta.active_part(),
                                          tCdfemSupport, tPhaseSupport, tLevelSetFields);

    tAuxMeta.clear_force_64bit_flag();
    ::krino::CDMesh::decompose_mesh(aBackgroundMesh.mKrinoMesh->bulk_data(), *tInterfaceGeometry);
}

auto get_level_set_field_name() -> std::string { return std::string{kLevelSetName}; }

auto get_level_set_values(const BackgroundMesh& aBackgroundMesh) -> std::unordered_map<stk::mesh::EntityId, double>
{
    const auto tNodes = node_entities_in_mesh(aBackgroundMesh);

    auto tLevelSetValues =
        utilities::reserved_container<std::unordered_map<stk::mesh::EntityId, double>>(tNodes.size());

    std::ranges::transform(tNodes, std::inserter(tLevelSetValues, tLevelSetValues.begin()),
                           [&aBackgroundMesh](const auto& aNode)
                           {
                               const auto tId = aBackgroundMesh.mKrinoMesh->bulk_data().entity_key(aNode).id();
                               return std::make_pair(tId, level_set_value(aBackgroundMesh.mLevelSetFields, aNode));
                           });

    return utilities::reduce_map(tLevelSetValues, retrieve_mpi_communicator_from_krino());
}

auto node_entities_in_mesh(const BackgroundMesh& aBackgroundMesh) -> stk::mesh::EntityVector
{
    const ::krino::CDFEM_Support& tCdfemSupport = ::krino::CDFEM_Support::get(aBackgroundMesh.mKrinoMesh->meta_data());
    const stk::mesh::Selector tSelector =
        stk::mesh::selectField(aBackgroundMesh.mLevelSetFields.front().isovar) & !tCdfemSupport.get_child_node_part();
    stk::mesh::EntityVector tNodes;
    stk::mesh::get_selected_entities(
        tSelector, aBackgroundMesh.mKrinoMesh->bulk_data().buckets(stk::topology::NODE_RANK), tNodes, kSortByGlobalId);
    return tNodes;
}

auto create_output_selector(stk::mesh::BulkData& aBulkData, const VoidPhase aVoidPhase) -> stk::mesh::Selector
{
    const auto tOutputSelector = output_selector(
        aBulkData.mesh_meta_data(), ::krino::AuxMetaData::get(aBulkData.mesh_meta_data()).active_part(), aVoidPhase);

    ::krino::fix_face_and_edge_ownership_to_assure_selected_owned_element(aBulkData, tOutputSelector);
    ::krino::fix_node_ownership_to_assure_selected_owned_element(aBulkData, tOutputSelector);

    return tOutputSelector;
}

auto void_phase_name() -> std::string_view { return kVoidPhaseName; }

void write_mesh(const stk::mesh::BulkData& aBulkData,
                const std::filesystem::path& aOutputFileName,
                const stk::mesh::Selector& aSelector)
{
    constexpr auto tStepIndex = 1;
    constexpr auto tTime = 0.0;
    ::krino::output_composed_mesh_with_fields(aBulkData, aSelector, aOutputFileName.string(), tStepIndex, tTime,
                                              stk::io::WRITE_RESULTS, kLevelSetFields);
}

}  // namespace plato::third_party_integration::krino
