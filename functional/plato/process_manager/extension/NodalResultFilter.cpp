#include "plato/process_manager/extension/NodalResultFilter.hpp"

#include "plato/filter/extension/kernel_filters/CanonicalKernelFilter.hpp"
#include "plato/filter/extension/kernel_filters/KernelFilter.hpp"
#include "plato/geometry/extension/DensityTopology.hpp"
#include "plato/input_parser/ComponentParserRegistration.hpp"
#include "plato/input_parser/InputBlockUtilities.hpp"
#include "plato/input_validation/ValidationRegistration.hpp"
#include "plato/mesh/DesignVariableConversion.hpp"
#include "plato/mesh/EntityCounts.hpp"
#include "plato/mesh/EntityRetrieval.hpp"
#include "plato/mesh/FixedBlockUtilities.hpp"
#include "plato/mesh/MeshFieldAppender.hpp"
#include "plato/mesh/MeshFieldWriter.hpp"
#include "plato/process_manager/library/ProcessManagerLogger.hpp"
#include "plato/services/SystemLogger.hpp"
#include "plato/services/TaskLogSetupTeardown.hpp"
#include "plato/utilities/FileUtilities.hpp"
#include "plato/utilities/RankSplitVector.hpp"
#include "plato/utilities/StringUtilities.hpp"

namespace plato::process_manager::extension
{
namespace
{
constexpr auto kRootRank = 0;

[[nodiscard]] auto make_nodal_result_filter_process_manager(const library::ValidatedProcessManagerInput& aValidInput)
    -> library::StageAndProcessManager
{
    return {library::RunStage::kPostProcess,
            [aValidInput](const library::ProcessManagerData&) { NodalResultFilter{aValidInput}.run(); }};
}

[[maybe_unused]] static const auto kElementToNodeParserRegistration =
    input_parser::ComponentParserRegistration<input_parser::nodal_result_filter>{};

[[maybe_unused]] static const auto kElementToNodeProcessManagerRegistration =
    library::ProcessManagerRegistration{input_parser::block_name<input_parser::nodal_result_filter>(),
                                        [](const library::ValidatedProcessManagerInput& aValidInput)
                                        { return make_nodal_result_filter_process_manager(aValidInput); }};

[[maybe_unused]] static const auto kElementToNodeValidationRegistration =
    input_validation::InputBlockValidationRegistration<>{
        [](const input_parser::nodal_result_filter& aInput)
        { return detail::validate_filter_is_kernel_filter(aInput); },
        [](const input_parser::nodal_result_filter& aInput)
        { return detail::validate_geometry_is_density_topology(aInput); }};

[[nodiscard]] auto filter_input(const library::ValidatedProcessManagerInput& aInput)
    -> const input_parser::kernel_filter&
{
    return input_validation::get_input_block<input_parser::nodal_result_filter>(aInput)
        .filter->mInputBlock.get<input_parser::kernel_filter>();
}

[[nodiscard]] auto mesh_input_path(const library::ValidatedProcessManagerInput& aInput) -> std::filesystem::path
{
    const auto tElementToNodeFilterInput = input_validation::get_input_block<input_parser::nodal_result_filter>(aInput);
    return tElementToNodeFilterInput.geometry->mInputBlock.get<input_parser::density_topology>().output_name->mToken;
}

[[nodiscard]] auto mesh_output_path(const library::ValidatedProcessManagerInput& aInput) -> std::filesystem::path
{
    const auto tElementToNodeFilterInput = input_validation::get_input_block<input_parser::nodal_result_filter>(aInput);
    if (tElementToNodeFilterInput.output_file_name)
    {
        return tElementToNodeFilterInput.output_file_name->mToken;
    }
    return tElementToNodeFilterInput.geometry->mInputBlock.get<input_parser::density_topology>().output_name->mToken;
}

[[nodiscard]] auto fixed_blocks(const library::ValidatedProcessManagerInput& aInput) -> std::set<std::string>
{
    const auto tElementToNodeFilterInput = input_validation::get_input_block<input_parser::nodal_result_filter>(aInput);
    return mesh::fixed_blocks(tElementToNodeFilterInput.geometry->mInputBlock.get<input_parser::density_topology>());
}

/// @brief Splits @a aComm into one or two groups.
///
/// If @a aNumberOfProcessors is equal to the number of ranks on comm world, then all ranks are allocated to the same
/// group. If it is smaller, then two groups are formed, one with @a aNumberOfProcessors and one with the remainder.
[[nodiscard]] auto split_comm(const unsigned int aNumberOfProcessors, const boost::mpi::communicator& aComm)
    -> std::pair<boost::mpi::communicator, utilities::ColorType>
{
    const auto tCommunicatorSize = aComm.size();
    const auto tGroupColor = utilities::rank_group_color({aNumberOfProcessors, tCommunicatorSize - aNumberOfProcessors},
                                                         utilities::RankNamedType{aComm.rank()});
    return {aComm.split(tGroupColor.mValue), tGroupColor.mValue};
}

auto make_mesh_writer(const mesh::OutputMode aOutputMode,
                      const mesh::Mesh& aOriginalMesh,
                      const std::filesystem::path& aOutputMeshPath,
                      const std::set<std::string>& aFixedBlocks,
                      const double aTimeStep) -> std::unique_ptr<mesh::MeshOutput>
{
    if (aOutputMode == mesh::OutputMode::kOverwrite)
    {
        return std::make_unique<mesh::MeshFieldWriter>(aOriginalMesh, aOutputMeshPath, aTimeStep);
    }
    return std::make_unique<mesh::MeshFieldAppender>(mesh::Mesh{aOutputMeshPath, aFixedBlocks}, aTimeStep);
}

void write_nodal_filtered_results(
    const mesh::Mesh& aMesh,
    const filter::extension::kernel_filters::KernelFilter<input_parser::kernel_filter>& aFilter,
    const std::filesystem::path& aOutputMeshPath,
    const std::set<std::string>& aFixedBlocks,
    const boost::mpi::communicator& aComm)
{
    const auto tTimeSteps = mesh::EntityCounts{aMesh}.timeSteps();
    const auto tTemporaryOutputPath = utilities::make_filename_unique(aOutputMeshPath);
    for (const auto tTimeStep : tTimeSteps)
    {
        const auto tNodalFieldToFilter = mesh::EntityRetrieval{aMesh}.designDomainNodalField(
            geometry::extension::density_mesh_field_name(), tTimeStep);
        const auto tFieldAnalysisMesh = mesh::DesignVariablesConversion{aMesh}.nodalFieldToAnalysisDomainMesh(
            mesh::NodalFieldVectorReference{tNodalFieldToFilter});
        const auto tFilteredField = aFilter.filter(tFieldAnalysisMesh);
        if (aComm.rank() == kRootRank)
        {
            constexpr auto tFixedValue = geometry::extension::density_fixed_value();
            const auto tMode =
                tTimeStep == tTimeSteps.front() ? mesh::OutputMode::kOverwrite : mesh::OutputMode::kAppend;

            const auto tWriter = make_mesh_writer(tMode, aMesh, tTemporaryOutputPath, aFixedBlocks, tTimeStep);

            tWriter->addFieldFromAnalysisDomainMesh(tFilteredField, NodalResultFilter::field_name(), tFixedValue);

            tWriter->addFieldFromAnalysisDomainMesh(tFieldAnalysisMesh, geometry::extension::density_mesh_field_name(),
                                                    tFixedValue);
        }
        aComm.barrier();
    }
    if (aComm.rank() == kRootRank)
    {
        std::filesystem::copy_file(tTemporaryOutputPath, aOutputMeshPath,
                                   std::filesystem::copy_options::overwrite_existing);
        std::filesystem::remove(tTemporaryOutputPath);
    }
}
}  // namespace

NodalResultFilter::NodalResultFilter(const library::ValidatedProcessManagerInput& aInput)
    : mInputMeshPath{mesh_input_path(aInput)},
      mOutputMeshPath{mesh_output_path(aInput)},
      mFixedBlockNames{fixed_blocks(aInput)},
      mFilterRadius{filter_input(aInput).filter_radius.value()},
      mNumberOfProcessorsForFilter{filter_input(aInput).number_of_processors.value_or(1U)}
{
}

void NodalResultFilter::run() const
{
    namespace fek = filter::extension::kernel_filters;
    [[maybe_unused]] const auto tTaskLogger = library::run_task_log<input_parser::nodal_result_filter>();

    const auto tMesh = mesh::Mesh{mInputMeshPath, mFixedBlockNames};
    const auto tWorldCommunicator = boost::mpi::communicator{};
    if (mesh::EntityCounts{tMesh}.hasNodalFieldVariable(geometry::extension::density_mesh_field_name()))
    {
        const auto [tCommunicator, tGroupColor] = split_comm(mNumberOfProcessorsForFilter, tWorldCommunicator);
        if (tGroupColor == 0)
        {
            const auto tKernelFilterType = fek::detail::make_kernel_filter_type(
                mFilterRadius, input_parser::KernelFilterCenteringTypes::kNodeCentered, tMesh);
            const auto tFilter = fek::KernelFilter<input_parser::kernel_filter>{
                fek::SourceMesh{tMesh}, fek::TargetMesh{tMesh}, tKernelFilterType, tCommunicator};
            write_nodal_filtered_results(tMesh, tFilter, mOutputMeshPath, mFixedBlockNames, tCommunicator);
        }
        tWorldCommunicator.barrier();
    }
    else if (tWorldCommunicator.rank() == kRootRank)
    {
        auto tLogger = library::process_manager_logger<input_parser::nodal_result_filter>();
        tLogger.logWarning(utilities::concatenate("Could not find field with name ",
                                                  geometry::extension::density_mesh_field_name(), " in mesh ",
                                                  mInputMeshPath, "\nNo filtered output will be added."));
    }
}

auto NodalResultFilter::field_name() -> std::string_view { return "nodal_filtered_result"; }

namespace detail
{
auto validate_filter_is_kernel_filter(const input_parser::nodal_result_filter& aInput) -> std::optional<std::string>
{
    return validate_expected_cross_reference_type<input_parser::kernel_filter>(aInput.filter);
}

auto validate_geometry_is_density_topology(const input_parser::nodal_result_filter& aInput)
    -> std::optional<std::string>
{
    return validate_expected_cross_reference_type<input_parser::density_topology>(aInput.geometry);
}

}  // namespace detail
}  // namespace plato::process_manager::extension
