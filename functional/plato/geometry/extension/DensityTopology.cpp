#include "plato/geometry/extension/DensityTopology.hpp"

#include <boost/mpi/communicator.hpp>
#include <optional>
#include <variant>

#include "plato/analysis/AnalysisDomainMeshSequentialView.hpp"
#include "plato/filter/library/FilterFactory.hpp"
#include "plato/filter/library/FilterInterface.hpp"
#include "plato/filter/library/FilterJacobian.hpp"
#include "plato/filter/library/FilterRegistration.hpp"
#include "plato/geometry/extension/FixedBlockUtilities.hpp"
#include "plato/geometry/extension/MeshValidationUtilities.hpp"
#include "plato/geometry/extension/OutputUtilities.hpp"
#include "plato/geometry/library/GeometryFilterUtilities.hpp"
#include "plato/geometry/library/GeometryLogger.hpp"
#include "plato/geometry/library/GeometryRegistration.hpp"
#include "plato/geometry/library/GeometryValidation.hpp"
#include "plato/geometry/library/OutputInfo.hpp"
#include "plato/input_parser/ComponentParserRegistration.hpp"
#include "plato/input_validation/ValidationRegistration.hpp"
#include "plato/input_validation/ValidationUtilities.hpp"
#include "plato/mesh/DesignVariableConversion.hpp"
#include "plato/mesh/EntityCounts.hpp"
#include "plato/mesh/Mesh.hpp"
#include "plato/mesh/MeshBlocks.hpp"
#include "plato/mesh/MeshFieldAppender.hpp"
#include "plato/mesh/MeshFieldWriter.hpp"
#include "plato/services/TaskLogSetupTeardown.hpp"

namespace plato::geometry::extension
{
namespace
{
constexpr double kDensityLowerBound = 0.0;
constexpr double kDensityUpperBound = 1.0;

constexpr auto kMeshNameAccessor = [](const input_parser::density_topology& aInput) { return aInput.mesh_name; };

[[nodiscard]] auto output_name(const input_parser::density_topology& aInput) -> const std::string&
{
    assert(aInput.output_name.has_value());
    return aInput.output_name.value().mToken;
}

[[nodiscard]] auto make_topology_output(const library::ValidatedGeometryInput& aGeometryInput)
    -> library::FactoryTypes::Output
{
    return
        [aGeometryInput](const linear_algebra::DynamicVector<double>& aSolution, const library::OutputInfo& aOutputInfo)
    {
        const auto tFilter = library::make_filter_from_geometry_input<input_parser::density_topology>(aGeometryInput);
        const auto& tInput = input_validation::get_input_block<input_parser::density_topology>(aGeometryInput);
        return DensityTopology::output(aSolution, tFilter, tInput, aOutputInfo);
    };
}

[[nodiscard]] auto make_topology_geometry(const library::ValidatedGeometryInput& aGeometryInput)
    -> library::GeometryFunction
{
    const auto& tInput = input_validation::get_input_block<input_parser::density_topology>(aGeometryInput);
    const auto tDensityTopology = std::make_shared<DensityTopology>(
        tInput, library::make_filter_from_geometry_input<input_parser::density_topology>(aGeometryInput));
    return library::GeometryFunction{[tDensityTopology](const linear_algebra::DynamicVector<double>& x)
                                     { return tDensityTopology->generateMesh(x); },
                                     [tDensityTopology](const linear_algebra::DynamicVector<double>& x)
                                     { return tDensityTopology->jacobian(x); },
                                     [tDensityTopology](const linear_algebra::DynamicVector<double>& x)
                                     { return tDensityTopology->adjointJacobian(x); }};
}

/// Static registration for the input parser
[[maybe_unused]] static auto kDensityTopologyParserRegistration =
    input_parser::ComponentParserRegistration<input_parser::density_topology>{};

/// Static registration for library
[[maybe_unused]] static auto kDensityTopologyRegistration = plato::geometry::library::GeometryRegistration{
    input_parser::block_name<input_parser::density_topology>(),
    [](const library::ValidatedGeometryInput& aGeometryInput)
    {
        const auto& tInput = input_validation::get_input_block<input_parser::density_topology>(aGeometryInput);
        return library::FactoryTypes{make_topology_geometry(aGeometryInput), DensityTopology::initialGuess(tInput),
                                     DensityTopology::bounds(tInput), make_topology_output(aGeometryInput)};
    }};

/// Static registration for validation functions
[[maybe_unused]] static auto kDensityTopologyInputValidationRegistration =
    input_validation::InputBlockValidationRegistration<>{
        [](const input_parser::density_topology& aInput) { return library::detail::validate_mesh_name(aInput); },
        [](const input_parser::density_topology& aInput)
        { return library::validate_filter_with_mesh(aInput, kMeshNameAccessor); },
        [](const input_parser::density_topology& aInput) { return library::detail::validate_mesh_file_exists(aInput); },
        [](const input_parser::density_topology& aInput)
        { return validate_unique_fixed_block_names(aInput, kMeshNameAccessor); },
        [](const input_parser::density_topology& aInput)
        { return validate_fixed_block_names_exist(aInput, kMeshNameAccessor); },
        [](const input_parser::density_topology& aInput)
        { return validate_at_least_one_design_block(aInput, kMeshNameAccessor); },
        [](const input_parser::density_topology& aInput) { return library::detail::validate_output_name(aInput); },
        [](const input_parser::density_topology& aInput) { return detail::validate_initial_density_value(aInput); },
        [](const input_parser::density_topology& aInput) { return validate_initial_field_source(aInput); },
        [](const input_parser::density_topology& aInput)
        { return detail::validate_exactly_one_initial_topology_specifier(aInput); }};

}  // namespace

DensityTopology::DensityTopology(const input_parser::density_topology& aInput,
                                 plato::filter::library::FilterFunction aFilterFunction)
    : mMesh(mesh_from_input(aInput)), mFilter(std::move(aFilterFunction))
{
}

analysis::AnalysisDomainMesh DensityTopology::generateMesh(
    const linear_algebra::DynamicVector<double>& aDesignParameters) const
{
    [[maybe_unused]] const auto tTaskLogger = services::TaskLogSetupTeardown{
        "Generating densities", library::geometry_logger<input_parser::density_topology>()};

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
        {
            [[maybe_unused]] const auto tTaskLogger = services::TaskLogSetupTeardown{
                "Vector-Jacobian product", library::geometry_logger<input_parser::density_topology>()};

            return x * mFilter.evaluate<core::evaluation::kFirstDerivative>(tAnalysisDomainMesh);
        }};
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
            [[maybe_unused]] const auto tTaskLogger = services::TaskLogSetupTeardown{
                "Vector-adjoin-Jacobian product", library::geometry_logger<input_parser::density_topology>()};

            return x * mFilter.evaluate<core::evaluation::kFirstDerivative, core::MatrixOrdering::kAdjoint>(
                           tAnalysisDomainMesh);
        }}};
}

auto DensityTopology::initialGuess(const input_parser::density_topology& aInput)
    -> linear_algebra::DynamicVector<double>
{
    if (aInput.initial_density_value.has_value())
    {
        const auto tMesh = mesh_from_input(aInput);
        const unsigned int tNumNodes = mesh::EntityCounts{tMesh}.numberOfDesignDomainNodes();
        return linear_algebra::DynamicVector<double>(tNumNodes, aInput.initial_density_value.value());
    }
    return linear_algebra::DynamicVector<double>(initial_field_from_mesh(aInput));
}

auto DensityTopology::bounds(const input_parser::density_topology& aInput)
    -> std::pair<std::vector<double>, std::vector<double>>
{
    const auto tMesh = mesh::EntityCounts{mesh_from_input(aInput)};
    const unsigned int tNumNodes = tMesh.numberOfDesignDomainNodes();
    return {std::vector<double>(tNumNodes, kDensityLowerBound), std::vector<double>(tNumNodes, kDensityUpperBound)};
}

void DensityTopology::output(const linear_algebra::DynamicVector<double>& aSolution,
                             const filter::library::FilterFunction& aFilterFunction,
                             const input_parser::density_topology& aInput,
                             const library::OutputInfo& aOutputInfo)
{
    [[maybe_unused]] const auto tTaskLogger =
        services::TaskLogSetupTeardown{"Writing output", library::geometry_logger<input_parser::density_topology>()};

    const auto tMeshFieldOutput = MeshFieldOutputInfo{mesh_from_input(aInput),
                                                      output_name(aInput),
                                                      fixed_blocks(aInput),
                                                      density_mesh_field_name(),
                                                      filtered_density_mesh_field_name(),
                                                      density_fixed_value()};
    output_nodal_field(tMeshFieldOutput, aFilterFunction, aSolution, aOutputInfo);
}

namespace detail
{
auto validate_initial_density_value(const input_parser::density_topology& aInput) -> std::optional<std::string>
{
    namespace pfu = plato::utilities;
    return input_validation::error_message_for_optional_parameter_out_of_bounds(
        input_parser::block_name<input_parser::density_topology>(), aInput.initial_density_value,
        "initial_density_value", pfu::ParameterBounds{pfu::Exclusive{0.0}, pfu::Inclusive{1.0}});
}

auto validate_exactly_one_initial_topology_specifier(const input_parser::density_topology& aInput)
    -> std::optional<std::string>
{
    const bool tBothAreTrue = aInput.initial_field_name.has_value() && aInput.initial_density_value.has_value();
    const bool tBothAreFalse = !aInput.initial_field_name.has_value() && !aInput.initial_density_value.has_value();

    if (tBothAreFalse || tBothAreTrue)
    {
        return utilities::concatenate(input_parser::block_name<input_parser::density_topology>(),
                                      ": You must specify exactly one initial topology value. Either "
                                      "'initial_density_value' or 'initial_field_name'.");
    }
    return std::nullopt;
}

}  // namespace detail
}  // namespace plato::geometry::extension
