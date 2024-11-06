#include "plato/geometry/extension/DensityTopology.hpp"

#include <boost/mpi/communicator.hpp>
#include <optional>
#include <variant>

#include "plato/analysis/AnalysisDomainMeshSequentialView.hpp"
#include "plato/core/ValidationRegistration.hpp"
#include "plato/core/ValidationUtilities.hpp"
#include "plato/filter/library/FilterInterface.hpp"
#include "plato/filter/library/FilterJacobian.hpp"
#include "plato/filter/library/FilterRegistration.hpp"
#include "plato/geometry/library/GeometryRegistration.hpp"
#include "plato/geometry/library/GeometryValidation.hpp"
#include "plato/mesh/DesignVariableConversion.hpp"
#include "plato/mesh/EntityCounts.hpp"
#include "plato/mesh/EntityRetrieval.hpp"
#include "plato/mesh/Mesh.hpp"
#include "plato/mesh/MeshBlocks.hpp"
#include "plato/mesh/MeshFieldWriter.hpp"
#include "plato/utilities/StringUtilities.hpp"

namespace plato::geometry::extension
{
namespace
{

constexpr double kDensityLowerBound = 0.0;
constexpr double kDensityUpperBound = 1.0;
constexpr double kDensityFixedValue = 1.0;

constexpr auto kRestartFileNamePrefix = std::string_view{"restart_"};
constexpr auto kTopologyFieldName = std::string_view{"Density"};
constexpr auto kUnfilteredControlsFieldName = std::string_view{"UnfilteredDensity"};

[[nodiscard]] auto make_topology_output(const input_parser::density_topology& aInput)
    -> std::function<void(const linear_algebra::DynamicVector<double>&)>
{
    return [aInput](const linear_algebra::DynamicVector<double>& aSolution)
    { return DensityTopology::output(aSolution, aInput); };
}

[[nodiscard]] auto make_filter(const input_parser::density_topology& aInput) -> filter::library::FilterFunction
{
    return filter::library::make_filter_function(
        library::get_cross_referenced_filter<plato::filter::library::ValidatedFilterInput>(aInput));
}

[[nodiscard]] auto make_topology_geometry(const input_parser::density_topology& aInput) -> library::GeometryFunction
{
    const auto tDensityTopology = std::make_shared<DensityTopology>(aInput, make_filter(aInput));
    return library::GeometryFunction{[tDensityTopology](const linear_algebra::DynamicVector<double>& x)
                                     { return tDensityTopology->generateMesh(x); },
                                     [tDensityTopology](const linear_algebra::DynamicVector<double>& x)
                                     { return tDensityTopology->jacobian(x); },
                                     [tDensityTopology](const linear_algebra::DynamicVector<double>& x)
                                     { return tDensityTopology->adjointJacobian(x); }};
}

[[nodiscard]] bool filter_is_cross_linked(const input_parser::density_topology& aInput)
{
    return aInput.filter->mInputBlock.template holds_expected_type<plato::filter::library::FilterInput>();
}

/// Static registration for library
[[maybe_unused]] static auto kDensityTopologyRegistration = plato::geometry::library::GeometryRegistration{
    input_parser::block_name<input_parser::density_topology>(),
    [](const library::ValidatedGeometryInput& aGeometryInput)
    {
        const auto& tInput = core::validated_variant_raw_input<input_parser::density_topology>(aGeometryInput);
        return library::FactoryTypes{make_topology_geometry(tInput), DensityTopology::initialGuess(tInput),
                                     DensityTopology::bounds(tInput), make_topology_output(tInput)};
    }};

/// Static registration for input validation functions
[[maybe_unused]] static auto kDensityTopologyValidationRegistration =
    core::ValidationRegistration<input_parser::density_topology>{
        [](const input_parser::density_topology& aInput) { return library::detail::validate_mesh_name(aInput); },
        [](const input_parser::density_topology& aInput) { return detail::validate_filter_with_mesh(aInput); },
        [](const input_parser::density_topology& aInput) { return library::detail::validate_mesh_file_exists(aInput); },
        [](const input_parser::density_topology& aInput) { return detail::validate_unique_fixed_block_names(aInput); },
        [](const input_parser::density_topology& aInput) { return detail::validate_fixed_block_names_exist(aInput); },
        [](const input_parser::density_topology& aInput) { return detail::validate_at_least_one_design_block(aInput); },
        [](const input_parser::density_topology& aInput) { return detail::validate_output_name(aInput); },
        [](const input_parser::density_topology& aInput) { return detail::validate_initial_density_value(aInput); },
        [](const input_parser::density_topology& aInput) { return detail::validate_initial_topology_source(aInput); },
        [](const input_parser::density_topology& aInput)
        { return detail::validate_exactly_one_initial_topology_specifier(aInput); }};

std::vector<std::string> mesh_block_names(const input_parser::density_topology& aInput)
{
    if (!aInput.mesh_name.has_value() || !std::filesystem::exists(aInput.mesh_name.value().mToken))
    {
        return {};
    }
    return mesh::MeshBlocks{mesh::Mesh{aInput.mesh_name.value().mToken}}.blockNames();
}

std::string mesh_block_names_for_error_message(const input_parser::density_topology& aInput)
{
    if (const auto tBlockNames = mesh_block_names(aInput); !tBlockNames.empty())
    {
        auto tAllBlockNames = utilities::concatenate_container(tBlockNames, ", ");
        return utilities::concatenate(
            "fixed_block must be one or more of the following names found in the input mesh: ",
            std::move(tAllBlockNames));
    }
    return "No blocks found in the mesh, or mesh_file does not exist.";
}

std::string mesh_field_names_for_error_message(const input_parser::density_topology& aInput)
{
    const auto tMesh = detail::mesh_from_input(aInput);
    const auto tNodalFields = mesh::EntityRetrieval{tMesh}.nodalFields();

    return utilities::concatenate("Field name must be one of the following defined on the nodes: ",
                                  utilities::concatenate_container(tNodalFields, ", "), ".");
}

}  // namespace

DensityTopology::DensityTopology(const input_parser::density_topology& aInput,
                                 plato::filter::library::FilterFunction aFilterFunction)
    : mMesh(detail::mesh_from_input(aInput)), mFilter(std::move(aFilterFunction))
{
}

analysis::AnalysisDomainMesh DensityTopology::generateMesh(
    const linear_algebra::DynamicVector<double>& aDesignParameters) const
{
    const auto tNodalDesignParameters = mesh::DesignVariablesConversion{mMesh}.nodalFieldToAnalysisDomainMesh(
        mesh::NodalFieldVectorReference{aDesignParameters.stdVector()});
    return mFilter.evaluate<core::evaluation::kFunction>(tNodalDesignParameters);
}

linear_algebra::JacobianMultiplier DensityTopology::jacobian(
    const linear_algebra::DynamicVector<double>& aDesignParameters) const
{
    const auto tDesignVariableConverter = mesh::DesignVariablesConversion{mMesh};
    const auto tNodalDesignParameters = mesh::NodalFieldVectorReference{aDesignParameters.stdVector()};
    return linear_algebra::JacobianMultiplier{
        /*.mVectorTimesJacobianFunction=*/
        [tAnalysisDomainMesh = tDesignVariableConverter.nodalFieldToAnalysisDomainMesh(tNodalDesignParameters),
         this](const linear_algebra::DynamicVector<double>& x)
        { return x * mFilter.evaluate<core::evaluation::kFirstDerivative>(tAnalysisDomainMesh); }};
}

auto DensityTopology::adjointJacobian(const linear_algebra::DynamicVector<double>& aDesignParameters) const
    -> linear_algebra::AdjointJacobianMultiplier
{
    const auto tDesignVariableConverter = mesh::DesignVariablesConversion{mMesh};
    const auto tNodalDesignParameters = mesh::NodalFieldVectorReference{aDesignParameters.stdVector()};
    return linear_algebra::AdjointJacobianMultiplier{linear_algebra::JacobianMultiplier{
        /*.mVectorTimesJacobianFunction=*/
        [tAnalysisDomainMesh = tDesignVariableConverter.nodalFieldToAnalysisDomainMesh(tNodalDesignParameters),
         this](const linear_algebra::DynamicVector<double>& x)
        {
            return x * mFilter.evaluate<core::evaluation::kFirstDerivative, core::MatrixOrdering::kAdjoint>(
                           tAnalysisDomainMesh);
        }}};
}

linear_algebra::DynamicVector<double> DensityTopology::initialGuess(const input_parser::density_topology& aInput)
{
    if (aInput.initial_density_value.has_value())
    {
        const auto tMesh = detail::mesh_from_input(aInput);
        const unsigned int tNumNodes = mesh::EntityCounts{tMesh}.numberOfDesignDomainNodes();
        return linear_algebra::DynamicVector<double>(tNumNodes, aInput.initial_density_value.value());
    }
    return linear_algebra::DynamicVector<double>(detail::initial_density_value_from_mesh(aInput));
}

std::pair<std::vector<double>, std::vector<double>> DensityTopology::bounds(
    const input_parser::density_topology& aInput)
{
    const auto tMesh = mesh::EntityCounts{detail::mesh_from_input(aInput)};
    const unsigned int tNumNodes = tMesh.numberOfDesignDomainNodes();
    return {std::vector<double>(tNumNodes, kDensityLowerBound), std::vector<double>(tNumNodes, kDensityUpperBound)};
}

void DensityTopology::output(const linear_algebra::DynamicVector<double>& aSolution,
                             const input_parser::density_topology& aInput)
{
    const auto& tOutputMeshName = aInput.output_name->mToken;
    const auto tRestartMeshName = std::string{kRestartFileNamePrefix} + tOutputMeshName;
    const auto tMesh = detail::mesh_from_input(aInput);
    const auto tFilter = make_filter(aInput);
    const auto tNodalDesignParameters = mesh::DesignVariablesConversion{tMesh}.nodalFieldToAnalysisDomainMesh(
        mesh::NodalFieldVectorReference{aSolution.stdVector()});

    if (boost::mpi::communicator{}.rank() == 0)
    {
        mesh::MeshFieldWriter{tMesh}.writeAnalysisDomainMesh(
            tOutputMeshName, tFilter.evaluate<core::evaluation::kFunction>(tNodalDesignParameters), kTopologyFieldName,
            kDensityFixedValue);
        mesh::MeshFieldWriter{tMesh}.writeAnalysisDomainMesh(tRestartMeshName, tNodalDesignParameters,
                                                             kUnfilteredControlsFieldName, kDensityFixedValue);
    }
}

namespace detail
{
std::optional<std::string> validate_output_name(const input_parser::density_topology& aInput)
{
    return core::error_message_for_empty_parameter(input_parser::block_name<input_parser::density_topology>(),
                                                   aInput.output_name, "output_name");
}

std::optional<std::string> validate_filter_with_mesh(const input_parser::density_topology& aInput)
{
    if (!aInput.filter || !filter_is_cross_linked(aInput))
    {
        return std::nullopt;
    }

    std::vector<std::string> tCurrentMessageList{};
    tCurrentMessageList = std::visit(
        [&aInput, tList = std::move(tCurrentMessageList)](const auto& aVariant) mutable -> std::vector<std::string>
        {  // NOLINTNEXTLINE
            return core::validate(aVariant, std::move(tList), std::filesystem::path{aInput.mesh_name.value().mToken});
        },
        library::get_cross_referenced_filter<plato::filter::library::FilterInput>(aInput));

    if (!tCurrentMessageList.empty())
    {
        return utilities::concatenate_container(tCurrentMessageList, "\n");
    }
    return std::nullopt;
}

std::optional<std::string> validate_unique_fixed_block_names(const input_parser::density_topology& aInput)
{
    if (!aInput.fixed_blocks.has_value())
    {
        return {};
    }

    const auto tUniqueFixedBlocks = fixed_blocks(aInput);
    if (tUniqueFixedBlocks.size() != aInput.fixed_blocks.value().mList.size())
    {
        auto tFixedBlockNames = utilities::concatenate_container(aInput.fixed_blocks.value().mList, ", ");
        auto tErrorMessage = utilities::concatenate(
            "The fixed_block entries in density_topology are not unique: ", std::move(tFixedBlockNames), ". ");
        return std::optional{std::move(tErrorMessage) + mesh_block_names_for_error_message(aInput)};
    }
    return {};
}

std::optional<std::string> validate_fixed_block_names_exist(const input_parser::density_topology& aInput)
{
    if (!aInput.fixed_blocks.has_value())
    {
        return {};
    }
    const auto tMeshBlockNames = mesh_block_names(aInput);
    const auto tUniqueFixedBlocks = fixed_blocks(aInput);
    auto tMissingFixedBlocks = std::vector<std::string>{};
    for (const auto& tInputBlockName : tUniqueFixedBlocks)
    {
        if (const auto tMeshBlockIter = std::find(tMeshBlockNames.cbegin(), tMeshBlockNames.cend(), tInputBlockName);
            tMeshBlockIter == tMeshBlockNames.cend())
        {
            tMissingFixedBlocks.push_back(tInputBlockName);
        }
    }
    if (!tMissingFixedBlocks.empty())
    {
        auto tAllMissingFixedBlockNames = utilities::concatenate_container(tMissingFixedBlocks, ", ");
        return std::optional{utilities::concatenate(
            "The following fixed_block entries could not be found in the mesh: ", std::move(tAllMissingFixedBlockNames),
            ". ", mesh_block_names_for_error_message(aInput))};
    }
    return std::nullopt;
}

std::optional<std::string> validate_at_least_one_design_block(const input_parser::density_topology& aInput)
{
    if (!aInput.fixed_blocks.has_value())
    {
        return {};
    }
    const auto tMeshBlockNames = mesh_block_names(aInput);
    const auto tUniqueFixedBlocks = fixed_blocks(aInput);
    if (tMeshBlockNames.size() == tUniqueFixedBlocks.size())
    {
        return std::optional{"All blocks have been listed under fixed_block, there is no design domain."};
    }
    return std::nullopt;
}

std::optional<std::string> validate_initial_density_value(const input_parser::density_topology& aInput)
{
    namespace pfu = plato::utilities;
    return core::error_message_for_optional_parameter_out_of_bounds(
        input_parser::block_name<input_parser::density_topology>(), aInput.initial_density_value,
        "initial_density_value", pfu::ParameterBounds{pfu::Exclusive{0.0}, pfu::Inclusive{1.0}});
}

std::optional<std::string> validate_initial_topology_source(const input_parser::density_topology& aInput)
{
    if (aInput.initial_density_field_name.has_value())
    {
        const auto tFieldName = aInput.initial_density_field_name.value().mToken;
        if (aInput.mesh_name && !std::filesystem::exists(aInput.mesh_name.value().mToken))
        {
            return utilities::concatenate(input_parser::block_name<input_parser::density_topology>(), ": The mesh  ",
                                          aInput.mesh_name.value().mToken, " does not exist.");
        }

        const auto tFileName = aInput.mesh_name.value().mToken;
        const bool tNodalFieldExists =
            mesh::EntityCounts{detail::mesh_from_input(aInput)}.hasNodalFieldVariable(std::string{tFieldName});

        if (!tNodalFieldExists)
        {
            return utilities::concatenate(
                input_parser::block_name<input_parser::density_topology>(), ": The mesh  ", tFileName,
                " does not have a nodal field called '", tFieldName,
                "'.\nYou must either specify an 'initial_density_value' or have the field defined on the mesh.\n",
                mesh_field_names_for_error_message(aInput));
        }
    }
    return std::nullopt;
}

std::optional<std::string> validate_exactly_one_initial_topology_specifier(const input_parser::density_topology& aInput)
{
    const bool tBothAreTrue = aInput.initial_density_field_name.has_value() && aInput.initial_density_value.has_value();
    const bool tBothAreFalse =
        !aInput.initial_density_field_name.has_value() && !aInput.initial_density_value.has_value();

    if (tBothAreFalse || tBothAreTrue)
    {
        return utilities::concatenate(input_parser::block_name<input_parser::density_topology>(),
                                      ": You must specify exactly one initial topology value. Either "
                                      "'initial_density_value' or 'initial_density_field_name'.");
    }
    return std::nullopt;
}

std::set<std::string> fixed_blocks(const input_parser::density_topology& aInput)
{
    if (!aInput.fixed_blocks.has_value())
    {
        return {};
    }
    auto tUniqueFixedBlocks = std::set<std::string>{};
    const auto& tRawFixedBlockInput = aInput.fixed_blocks.value().mList;
    std::copy(tRawFixedBlockInput.cbegin(), tRawFixedBlockInput.cend(),
              std::inserter(tUniqueFixedBlocks, tUniqueFixedBlocks.begin()));
    return tUniqueFixedBlocks;
}

mesh::Mesh mesh_from_input(const input_parser::density_topology& aInput)
{
    assert(aInput.mesh_name.has_value());
    return mesh::Mesh{aInput.mesh_name.value().mToken, fixed_blocks(aInput)};
}

auto initial_density_value_from_mesh(const input_parser::density_topology& aInput) -> std::vector<double>
{
    const auto tFieldName = aInput.initial_density_field_name.value().mToken;
    const auto tMesh = detail::mesh_from_input(aInput);

    return mesh::EntityRetrieval{tMesh}.designDomainNodalField(tFieldName);
}

}  // namespace detail
}  // namespace plato::geometry::extension
