#include "plato/geometry/extension/DensityTopology.hpp"

#include <boost/mpi/communicator.hpp>
#include <optional>
#include <variant>

#include "plato/analysis/AnalysisDomainMeshSequentialView.hpp"
#include "plato/core/ValidationRegistration.hpp"
#include "plato/core/ValidationUtilities.hpp"
#include "plato/filter/library/FilterFactory.hpp"
#include "plato/filter/library/FilterInterface.hpp"
#include "plato/filter/library/FilterJacobian.hpp"
#include "plato/filter/library/FilterRegistration.hpp"
#include "plato/geometry/extension/FixedBlockUtilities.hpp"
#include "plato/geometry/extension/MeshValidationUtilities.hpp"
#include "plato/geometry/extension/OutputUtilities.hpp"
#include "plato/geometry/library/GeometryFilterUtilities.hpp"
#include "plato/geometry/library/GeometryRegistration.hpp"
#include "plato/geometry/library/GeometryValidation.hpp"
#include "plato/geometry/library/OutputInfo.hpp"
#include "plato/mesh/DesignVariableConversion.hpp"
#include "plato/mesh/EntityCounts.hpp"
#include "plato/mesh/Mesh.hpp"
#include "plato/mesh/MeshBlocks.hpp"
#include "plato/mesh/MeshFieldAppender.hpp"
#include "plato/mesh/MeshFieldWriter.hpp"

namespace plato::geometry::extension
{
namespace
{
constexpr double kDensityLowerBound = 0.0;
constexpr double kDensityUpperBound = 1.0;
constexpr double kDensityFixedValue = 1.0;

constexpr auto kMeshNameAccessor = [](const input_parser::density_topology& aInput) { return aInput.mesh_name; };

[[nodiscard]] auto output_name(const input_parser::density_topology& aInput) -> const std::string&
{
    assert(aInput.output_name.has_value());
    return aInput.output_name.value().mToken;
}

[[nodiscard]] auto make_topology_output(const input_parser::density_topology& aInput) -> library::FactoryTypes::Output
{
    return [aInput](const linear_algebra::DynamicVector<double>& aSolution, const library::OutputInfo& aOutputInfo)
    {
        const auto tFilter = library::make_filter_from_geometry_input(aInput);
        return DensityTopology::output(aSolution, tFilter, aInput, aOutputInfo);
    };
}

[[nodiscard]] auto make_topology_geometry(const input_parser::density_topology& aInput) -> library::GeometryFunction
{
    const auto tDensityTopology =
        std::make_shared<DensityTopology>(aInput, library::make_filter_from_geometry_input(aInput));
    return library::GeometryFunction{[tDensityTopology](const linear_algebra::DynamicVector<double>& x)
                                     { return tDensityTopology->generateMesh(x); },
                                     [tDensityTopology](const linear_algebra::DynamicVector<double>& x)
                                     { return tDensityTopology->jacobian(x); },
                                     [tDensityTopology](const linear_algebra::DynamicVector<double>& x)
                                     { return tDensityTopology->adjointJacobian(x); }};
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
        [](const input_parser::density_topology& aInput)
        {
            return library::validate_filter_with_mesh(
                aInput, [](const auto& aDensityTopology) { return aDensityTopology.mesh_name; });
        },
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
        const auto tMesh = mesh_from_input(aInput);
        const unsigned int tNumNodes = mesh::EntityCounts{tMesh}.numberOfDesignDomainNodes();
        return linear_algebra::DynamicVector<double>(tNumNodes, aInput.initial_density_value.value());
    }
    return linear_algebra::DynamicVector<double>(initial_field_from_mesh(aInput));
}

std::pair<std::vector<double>, std::vector<double>> DensityTopology::bounds(
    const input_parser::density_topology& aInput)
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
    const auto tMeshFieldOutput = MeshFieldOutputInfo{mesh_from_input(aInput),
                                                      output_name(aInput),
                                                      fixed_blocks(aInput),
                                                      density_mesh_field_name(),
                                                      filtered_density_mesh_field_name(),
                                                      kDensityFixedValue};
    output_nodal_field(tMeshFieldOutput, aFilterFunction, aSolution, aOutputInfo);
}

namespace detail
{

std::optional<std::string> validate_initial_density_value(const input_parser::density_topology& aInput)
{
    namespace pfu = plato::utilities;
    return core::error_message_for_optional_parameter_out_of_bounds(
        input_parser::block_name<input_parser::density_topology>(), aInput.initial_density_value,
        "initial_density_value", pfu::ParameterBounds{pfu::Exclusive{0.0}, pfu::Inclusive{1.0}});
}

std::optional<std::string> validate_exactly_one_initial_topology_specifier(const input_parser::density_topology& aInput)
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
