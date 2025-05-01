#include "plato/process_manager/extension/NodalResultFilter.hpp"

#include "plato/filter/extension/KernelFilter.hpp"
#include "plato/geometry/extension/DensityTopology.hpp"
#include "plato/geometry/extension/FixedBlockUtilities.hpp"
#include "plato/input_parser/ComponentParserRegistration.hpp"
#include "plato/input_parser/InputBlockUtilities.hpp"
#include "plato/input_validation/ValidationRegistration.hpp"
#include "plato/mesh/DesignVariableConversion.hpp"
#include "plato/mesh/EntityCounts.hpp"
#include "plato/mesh/EntityRetrieval.hpp"
#include "plato/mesh/MeshFieldAppender.hpp"
#include "plato/mesh/MeshFieldWriter.hpp"
#include "plato/utilities/FileUtilities.hpp"
#include "plato/utilities/RankSplitVector.hpp"

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
    return geometry::extension::fixed_blocks(
        tElementToNodeFilterInput.geometry->mInputBlock.get<input_parser::density_topology>());
}

/// @brief Splits @a aComm into one or two groups.
///
/// If @a aNumberOfProcessors is equal to the number of ranks on comm world, then all ranks are allocated to the same
/// group. If it is smaller, then two groups are formed, one with @a aNumberOfProcessors and one with the remainder.
[[nodiscard]] auto split_comm(const unsigned int aNumberOfProcessors, const boost::mpi::communicator& aComm)
    -> std::pair<boost::mpi::communicator, utilities::ColorType>
{
    const auto tCommSize = aComm.size();
    const auto tGroupColor = utilities::rank_group_color({aNumberOfProcessors, tCommSize - aNumberOfProcessors},
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

void write_nodal_filtered_results(const mesh::Mesh& aMesh,
                                  const filter::extension::KernelFilter& aFilter,
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
            constexpr auto tFixedValue = double{1.0};
            const auto tMode =
                tTimeStep == tTimeSteps.front() ? mesh::OutputMode::kOverwrite : mesh::OutputMode::kAppend;
            const auto tWriter = make_mesh_writer(tMode, aMesh, tTemporaryOutputPath, aFixedBlocks, tTimeStep);
            tWriter->addFieldOnAnalysisDomainMesh(tFilteredField, NodalResultFilter::field_name(), tFixedValue);
            tWriter->addFieldOnAnalysisDomainMesh(tFieldAnalysisMesh, geometry::extension::density_mesh_field_name(),
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
    const auto tMesh = mesh::Mesh{mInputMeshPath, mFixedBlockNames};
    const auto tWorldComm = boost::mpi::communicator{};
    if (mesh::EntityCounts{tMesh}.hasNodalFieldVariable(geometry::extension::density_mesh_field_name()))
    {
        const auto [tComm, tGroupColor] = split_comm(mNumberOfProcessorsForFilter, tWorldComm);
        if (tGroupColor == 0)
        {
            const auto tFilter =
                filter::extension::KernelFilter{tMesh, filter::extension::FilterRadius{mFilterRadius},
                                                input_parser::KernelFilterCenteringTypes::kNodeCentered, tComm};
            write_nodal_filtered_results(tMesh, tFilter, mOutputMeshPath, mFixedBlockNames, tComm);
        }
        tWorldComm.barrier();
    }
    else if (tWorldComm.rank() == kRootRank)
    {
        std::cout << "Warning: " << input_parser::block_name<input_parser::nodal_result_filter>()
                  << " could not find field with name " << geometry::extension::density_mesh_field_name() << " in mesh "
                  << mInputMeshPath;
        std::cout << "\nNo filtered output will be added.\n";
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
