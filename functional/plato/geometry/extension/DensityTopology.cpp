#include "plato/geometry/extension/DensityTopology.hpp"

#include "plato/filter/library/FilterInterface.hpp"
#include "plato/filter/library/FilterJacobian.hpp"
#include "plato/geometry/library/GeometryRegistration.hpp"
#include "plato/geometry/library/GeometryValidation.hpp"
#include "plato/mesh/DesignVariableConversion.hpp"
#include "plato/mesh/EntityCounts.hpp"
#include "plato/mesh/Mesh.hpp"
#include "plato/mesh/MeshDesignVariablesViews.hpp"
#include "plato/third_party_integration/stk_io/Utilities.hpp"
#include "plato/utilities/Exception.hpp"

namespace plato::geometry::extension
{
namespace
{
constexpr double kInitialDensity = 0.5;
constexpr double kDensityLowerBound = 0.0;
constexpr double kDensityUpperBound = 1.0;

std::function<void(const linear_algebra::DynamicVector<double>&)> make_topology_output(
    const std::filesystem::path& aInputMeshName, const std::filesystem::path& aOutputMeshName)
{
    return [aInputMeshName, aOutputMeshName](const linear_algebra::DynamicVector<double>& aSolution)
    { return DensityTopology::output(aInputMeshName, aSolution, aOutputMeshName); };
}

/// Static registration for library
[[maybe_unused]] static auto kDensityTopologyRegistration = plato::geometry::library::GeometryRegistration{
    input_parser::block_name<input_parser::density_topology>(),
    [](const library::ValidatedGeometryInput& aGeometryInput)
    {
        const auto& tInput = core::validated_variant_raw_input<input_parser::density_topology>(aGeometryInput);
        return library::FactoryTypes{
            make_topology_geometry(DensityTopology{
                tInput, plato::filter::library::make_filter_function(library::get_cross_referenced_filter(tInput))}),
            DensityTopology::initialGuess(tInput), DensityTopology::bounds(tInput),
            make_topology_output(tInput.mesh_name.value().mToken, tInput.output_name.value().mToken)};
    }};

/// Static registration for input validation functions
[[maybe_unused]] static auto kDensityTopologyValidationRegistration =
    core::ValidationRegistration<input_parser::density_topology>{
        [](const input_parser::density_topology& aInput) { return library::detail::validate_mesh_name(aInput); },
        [](const input_parser::density_topology& aInput) { return detail::validate_output_name(aInput); }};

}  // namespace

DensityTopology::DensityTopology(const input_parser::density_topology& aInput,
                                 plato::filter::library::FilterFunction aFilterFunction)
    : mFileName(aInput.mesh_name.value().mToken),
      mMesh(detail::mesh_from_input(aInput)),
      mNumDesignParameters(mesh::EntityCounts{mMesh}.numberOfDesignDomainNodes()),
      mFilter(std::move(aFilterFunction))
{
}

mesh::MeshDesignVariables DensityTopology::generateMesh(
    const linear_algebra::DynamicVector<double>& aDesignParameters) const
{
    const auto tNodalDesignParameters = mesh::DesignVariablesConversion{mMesh}.nodalDensitiesToMeshDesignVariables(
        mesh::NodalDensityVectorReference{aDesignParameters.stdVector()});
    return mFilter.f(tNodalDesignParameters);
}

linear_algebra::JacobianMultiplier DensityTopology::jacobian(
    const linear_algebra::DynamicVector<double>& aDesignParameters) const
{
    const auto tDesignVariableConverter = mesh::DesignVariablesConversion{mMesh};
    const auto tNodalDesignParameters = mesh::NodalDensityVectorReference{aDesignParameters.stdVector()};
    return linear_algebra::JacobianMultiplier{
        /*.mNumColumns=*/mNumDesignParameters,
        /*.mJacobianTimesVectorFunction=*/
        [tMeshDesignVariables = tDesignVariableConverter.nodalDensitiesToMeshDesignVariables(tNodalDesignParameters),
         this](const linear_algebra::DynamicVector<double>& x) { return x * mFilter.df(tMeshDesignVariables); }};
}

linear_algebra::DynamicVector<double> DensityTopology::initialGuess(const input_parser::density_topology& aInput)
{
    const auto tMesh = detail::mesh_from_input(aInput);
    const unsigned int tNumNodes = mesh::EntityCounts{tMesh}.numberOfDesignDomainNodes();
    return linear_algebra::DynamicVector<double>(tNumNodes, kInitialDensity);
}

std::pair<std::vector<double>, std::vector<double>> DensityTopology::bounds(
    const input_parser::density_topology& aInput)
{
    const auto tMesh = detail::mesh_from_input(aInput);
    const unsigned int tNumNodes = mesh::EntityCounts{tMesh}.numberOfDesignDomainNodes();
    return {std::vector<double>(tNumNodes, kDensityLowerBound), std::vector<double>(tNumNodes, kDensityUpperBound)};
}

void DensityTopology::output(const std::filesystem::path& aInputMeshName,
                             const linear_algebra::DynamicVector<double>& aSolution,
                             const std::filesystem::path& aOutputMeshName)
{
    plato::third_party_integration::stk_io::write_nodal_density(aInputMeshName, aSolution.stdVector(), aOutputMeshName);
}

auto make_topology_geometry(const DensityTopology& aDensityTopology)
    -> core::Function<mesh::MeshDesignVariables,
                      linear_algebra::JacobianMultiplier,
                      const linear_algebra::DynamicVector<double>&>
{
    return core::make_function([tDensityTopology = aDensityTopology](const linear_algebra::DynamicVector<double>& x)
                               { return tDensityTopology.generateMesh(x); },
                               [tDensityTopology = aDensityTopology](const linear_algebra::DynamicVector<double>& x)
                               { return tDensityTopology.jacobian(x); });
}

namespace detail
{
std::optional<std::string> validate_output_name(const input_parser::density_topology& aInput)
{
    return core::error_message_for_empty_parameter(input_parser::block_name<input_parser::density_topology>(),
                                                   aInput.output_name, "output_name");
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
    return mesh::Mesh{aInput.mesh_name.value().mToken, fixed_blocks(aInput)};
}

}  // namespace detail
}  // namespace plato::geometry::extension
