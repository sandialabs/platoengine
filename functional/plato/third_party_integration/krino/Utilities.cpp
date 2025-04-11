#include "plato/third_party_integration/krino/Utilities.hpp"

#include <Akri_AuxMetaData.hpp>           // AuxMetaData::get
#include <Akri_CDFEM_Support.hpp>         //CDFEM_Support
#include <Akri_CDMesh.hpp>                //sierraTimer
#include <Akri_DiagWriter.hpp>            //initialize environment
#include <Akri_LevelSet.hpp>              //LevelSet
#include <Akri_LevelSetPolicy.hpp>        //LSPerInterfacePolicy
#include <Akri_MeshHelpers.hpp>           //activate_all_entities
#include <Akri_NodalSurfaceDistance.hpp>  //compute_nodal_surface_distance
#include <Akri_OutputUtils.hpp>
#include <boost/mpi/collectives.hpp>
#include <boost/mpi/communicator.hpp>
#include <boost/serialization/vector.hpp>
#include <iterator>
#include <stk_io/StkMeshIoBroker.hpp>  //get_selected_entities
#include <stk_mesh/base/Entity.hpp>
#include <stk_mesh/base/Types.hpp>
#include <stk_util/diag/WriterRegistry.hpp>    //initialize environment
#include <stk_util/environment/EnvData.hpp>    //initialize environment
#include <stk_util/environment/OutputLog.hpp>  //initialize environment
#include <string_view>

#include "plato/utilities/TransformIf.hpp"
#include "plato/utilities/Zip.hpp"

namespace plato::third_party_integration::krino
{
namespace
{
constexpr auto kSortByGlobalId = true;
constexpr auto kOutputDescription = std::string_view{"out>pout dout>out pout>null"};
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

auto read_and_setup_for_decomposition(const std::filesystem::path& aFilename) -> std::unique_ptr<::krino::MeshInterface>
{
    std::unique_ptr<::krino::MeshFromFile> tMeshFromFile = std::make_unique<::krino::MeshFromFile>(
        aFilename.string(), stk::EnvData::parallel_comm(), std::string{kDecompositionMethod});

    ::krino::LSPerInterfacePolicy::setup_levelsets_on_all_blocks_with_void_phase_for_any_negative_levelset(
        tMeshFromFile->meta_data(), kNumberOfLevelSets);
    ::krino::LevelSet& tLevelSet =
        ::krino::LevelSet::build(tMeshFromFile->meta_data(), std::string{kLevelSetName}, sierra::Diag::sierraTimer());
    tLevelSet.set_distance_name(std::string{kLevelSetName});  /// becomes set_levelset_field_name in Trilinos 16.1
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

namespace
{

[[nodiscard]] auto unique_merge_on_all_ranks(std::vector<stk::mesh::EntityId> aVector)
    -> std::vector<stk::mesh::EntityId>
{
    const auto tCommunicator = boost::mpi::communicator(
        reinterpret_cast<ompi_communicator_t*>(stk::EnvData::instance().m_parallelComm), boost::mpi::comm_duplicate);
    constexpr int tRootRank = 0;
    std::vector<std::vector<stk::mesh::EntityId>> tGatheredData;

    boost::mpi::gather(tCommunicator, aVector, tGatheredData, tRootRank);

    std::vector<stk::mesh::EntityId> tConcatenatedData;
    if (tCommunicator.rank() == tRootRank)
    {
        for (const auto& tSubData : tGatheredData)
        {
            tConcatenatedData.insert(tConcatenatedData.end(), tSubData.begin(), tSubData.end());
        }
        std::sort(tConcatenatedData.begin(), tConcatenatedData.end());
        auto tLastEntry = std::unique(tConcatenatedData.begin(), tConcatenatedData.end());
        tConcatenatedData.erase(tLastEntry, tConcatenatedData.end());
    }
    boost::mpi::broadcast(tCommunicator, tConcatenatedData, tRootRank);
    return tConcatenatedData;
}
}  // namespace

auto background_node_ids(const ::krino::MeshInterface& aKrinoMesh,
                         const std::vector<::krino::LS_Field>& aLevelSetFields) -> std::vector<stk::mesh::EntityId>
{
    const auto tBackgroundNodes = node_entities_in_mesh(aKrinoMesh, aLevelSetFields);
    return unique_merge_on_all_ranks(get_ids_from_entities(tBackgroundNodes, aKrinoMesh.bulk_data()));
}

auto cut_mesh_node_ids(const ::krino::MeshInterface& aKrinoMesh, const VoidPhase aVoidPhase)
    -> std::vector<stk::mesh::EntityId>
{
    const auto tSelector = output_selector(aKrinoMesh.meta_data(),
                                           ::krino::AuxMetaData::get(aKrinoMesh.meta_data()).active_part(), aVoidPhase);

    stk::mesh::EntityVector tNodes;
    stk::mesh::get_selected_entities(tSelector, aKrinoMesh.bulk_data().buckets(stk::topology::NODE_RANK), tNodes,
                                     kSortByGlobalId);

    return unique_merge_on_all_ranks(get_ids_from_entities(tNodes, aKrinoMesh.bulk_data()));
}

void cut_mesh(stk::mesh::BulkData& aBulkData, const std::vector<::krino::LS_Field>& aLevelSetFields)
{
    auto& tMeta = aBulkData.mesh_meta_data();
    auto& tAuxMeta = ::krino::AuxMetaData::get(tMeta);
    auto& tCdfemSupport = ::krino::CDFEM_Support::get(tMeta);
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

    return tLevelSetValues;
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

}  // namespace plato::third_party_integration::krino
