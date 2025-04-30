#include "plato/process_manager/extension/ElementToNodeResultFilter.hpp"

#include "plato/filter/extension/KernelFilter.hpp"
#include "plato/geometry/extension/DensityTopology.hpp"
#include "plato/geometry/extension/FixedBlockUtilities.hpp"
#include "plato/input_parser/ComponentParserRegistration.hpp"
#include "plato/input_parser/InputBlockUtilities.hpp"
#include "plato/input_validation/ValidationRegistration.hpp"
#include "plato/mesh/DesignVariableConversion.hpp"
#include "plato/mesh/EntityRetrieval.hpp"
#include "plato/mesh/MeshFieldAppender.hpp"
#include "plato/mesh/MeshFieldWriter.hpp"
#include "plato/utilities/RankSplitVector.hpp"

namespace plato::process_manager::extension
{
namespace
{
[[maybe_unused]] static const auto kElementToNodeParserRegistration =
    input_parser::ComponentParserRegistration<input_parser::element_to_node_result_filter>{};

[[maybe_unused]] static const auto kElementToNodeValidationRegistration =
    input_validation::InputBlockValidationRegistration<>{
        [](const input_parser::element_to_node_result_filter& aInput)
        { return detail::validate_filter_is_kernel_filter(aInput); },
        [](const input_parser::element_to_node_result_filter& aInput)
        { return detail::validate_geometry_is_density_topology(aInput); }};

[[nodiscard]] auto filter_input(const library::ValidatedProcessManagerInput& aInput)
    -> const input_parser::kernel_filter&
{
    return input_validation::get_input_block<input_parser::element_to_node_result_filter>(aInput)
        .filter->mInputBlock.get<input_parser::kernel_filter>();
}

[[nodiscard]] auto mesh_input_path(const library::ValidatedProcessManagerInput& aInput) -> std::filesystem::path
{
    const auto tElementToNodeFilterInput =
        input_validation::get_input_block<input_parser::element_to_node_result_filter>(aInput);
    return tElementToNodeFilterInput.geometry->mInputBlock.get<input_parser::density_topology>().output_name->mToken;
}

[[nodiscard]] auto mesh_output_path(const library::ValidatedProcessManagerInput& aInput) -> std::filesystem::path
{
    const auto tElementToNodeFilterInput =
        input_validation::get_input_block<input_parser::element_to_node_result_filter>(aInput);
    if (tElementToNodeFilterInput.output_file_name)
    {
        return tElementToNodeFilterInput.output_file_name->mToken;
    }
    return tElementToNodeFilterInput.geometry->mInputBlock.get<input_parser::density_topology>().output_name->mToken;
}

[[nodiscard]] auto fixed_blocks(const library::ValidatedProcessManagerInput& aInput) -> std::set<std::string>
{
    const auto tElementToNodeFilterInput =
        input_validation::get_input_block<input_parser::element_to_node_result_filter>(aInput);
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
}  // namespace

ElementToNodeResultFilter::ElementToNodeResultFilter(const library::ValidatedProcessManagerInput& aInput)
    : mInputMeshPath{mesh_input_path(aInput)},
      mOutputMeshPath{mesh_output_path(aInput)},
      mFixedBlockNames{fixed_blocks(aInput)},
      mFilterRadius{filter_input(aInput).filter_radius.value()},
      mNumberOfProcessorsForFilter{filter_input(aInput).number_of_processors.value()}
{
}

void ElementToNodeResultFilter::run(const library::ProcessManagerData& /*aProcessManagerData*/) const
{
    const auto tMesh = mesh::Mesh{mInputMeshPath, mFixedBlockNames};
    if (mesh::EntityRetrieval{tMesh}.hasNodalField(geometry::extension::density_mesh_field_name()))
    {
        const auto tNodalFieldToFilter =
            mesh::EntityRetrieval{tMesh}.designDomainNodalField(geometry::extension::density_mesh_field_name());
        const auto tFieldAnalysisMesh = mesh::DesignVariablesConversion{tMesh}.nodalFieldToAnalysisDomainMesh(
            mesh::NodalFieldVectorReference{tNodalFieldToFilter});
        const auto tWorldComm = boost::mpi::communicator{};
        const auto [tComm, tGroupColor] = split_comm(mNumberOfProcessorsForFilter, tWorldComm);
        if (tGroupColor == 0)
        {
            const auto tFilter =
                filter::extension::KernelFilter{tMesh, filter::extension::FilterRadius{mFilterRadius},
                                                input_parser::KernelFilterCenteringTypes::kNodeCentered, tComm};
            const auto tFilteredField = tFilter.filter(tFieldAnalysisMesh);
            if (tComm.rank() == 0)
            {
                constexpr auto tTimeStep = double{1.0};
                constexpr auto tFixedValue = double{1.0};
                auto tWriter = mesh::MeshFieldWriter{tMesh, mOutputMeshPath, tTimeStep};
                tWriter.addFieldOnAnalysisDomainMesh(tFilteredField, field_name(), tFixedValue);
                tWriter.addFieldOnAnalysisDomainMesh(tFieldAnalysisMesh, geometry::extension::density_mesh_field_name(),
                                                     tFixedValue);
            }
        }
        tWorldComm.barrier();
    }
    else
    {
        std::cout << "Warning: " << input_parser::block_name<input_parser::element_to_node_result_filter>()
                  << " could not find field with name " << geometry::extension::density_mesh_field_name() << " in mesh "
                  << mInputMeshPath;
        std::cout << "\nNo filtered output will be added.\n";
    }
}

auto ElementToNodeResultFilter::field_name() -> std::string_view { return "element_to_nodal_filtered_result"; }

namespace detail
{
auto validate_filter_is_kernel_filter(const input_parser::element_to_node_result_filter& aInput)
    -> std::optional<std::string>
{
    return validate_expected_cross_reference_type<input_parser::kernel_filter>(aInput.filter);
}

auto validate_geometry_is_density_topology(const input_parser::element_to_node_result_filter& aInput)
    -> std::optional<std::string>
{
    return validate_expected_cross_reference_type<input_parser::density_topology>(aInput.geometry);
}

}  // namespace detail
}  // namespace plato::process_manager::extension
