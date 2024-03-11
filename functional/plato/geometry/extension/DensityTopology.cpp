#include "plato/geometry/extension/DensityTopology.hpp"

#include "plato/filter/library/FilterInterface.hpp"
#include "plato/filter/library/FilterJacobian.hpp"
#include "plato/geometry/library/GeometryRegistration.hpp"
#include "plato/geometry/library/GeometryValidation.hpp"
#include "plato/utilities/Exception.hpp"
#include "plato/utilities/STKUtilities.hpp"

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
        const auto& tInput = library::geometry_raw_input<input_parser::density_topology>(aGeometryInput);
        return library::FactoryTypes{
            make_topology_geometry(DensityTopology{tInput}),
            DensityTopology::initialGuess(tInput.mesh_name.value().mName),
            DensityTopology::bounds(tInput.mesh_name.value().mName),
            make_topology_output(tInput.mesh_name.value().mName, tInput.output_name.value().mName)};
    }};

/// Static registration for input validation functions
[[maybe_unused]] static auto kDensityTopologyValidationRegistration =
    core::ValidationRegistration<input_parser::density_topology>{
        [](const input_parser::density_topology& aInput) { return library::detail::validate_mesh_name(aInput); },
        [](const input_parser::density_topology& aInput) { return detail::validate_output_name(aInput); }};
}  // namespace

DensityTopology::DensityTopology(const input_parser::density_topology& aInput)
    : mFileName(aInput.mesh_name.value().mName),
      mNumDesignParameters(plato::utilities::read_mesh_node_size(mFileName)),
      mFilter(plato::filter::library::make_filter_function(aInput))
{
}

core::MeshProxy DensityTopology::generateMesh(const linear_algebra::DynamicVector<double>& aDesignParameters) const
{
    return mFilter.f(core::MeshProxy{mFileName, aDesignParameters.stdVector()});
}

linear_algebra::JacobianMultiplier DensityTopology::jacobian(
    const linear_algebra::DynamicVector<double>& aDesignParameters) const
{
    return linear_algebra::JacobianMultiplier{/*.mNumColumns=*/mNumDesignParameters,
                                              /*.mJacobianTimesVectorFunction=*/
                                              [tMeshProxy = core::MeshProxy{mFileName, aDesignParameters.stdVector()},
                                               this](const linear_algebra::DynamicVector<double>& x)
                                              { return x * mFilter.df(tMeshProxy); }};
}

linear_algebra::DynamicVector<double> DensityTopology::initialGuess(const std::filesystem::path& aMeshFileName)
{
    const unsigned int tNumNodes = plato::utilities::read_mesh_node_size(aMeshFileName);
    return linear_algebra::DynamicVector<double>(tNumNodes, kInitialDensity);
}

std::pair<std::vector<double>, std::vector<double>> DensityTopology::bounds(const std::filesystem::path& aMeshFileName)
{
    const unsigned int tNumNodes = plato::utilities::read_mesh_node_size(aMeshFileName);
    return {std::vector<double>(tNumNodes, kDensityLowerBound), std::vector<double>(tNumNodes, kDensityUpperBound)};
}

void DensityTopology::output(const std::filesystem::path& aInputMeshName,
                             const linear_algebra::DynamicVector<double>& aSolution,
                             const std::filesystem::path& aOutputMeshName)
{
    plato::utilities::write_mesh_density(aInputMeshName, aSolution.stdVector(), aOutputMeshName);
}

auto make_topology_geometry(const DensityTopology& aDensityTopology)
    -> core::Function<core::MeshProxy, linear_algebra::JacobianMultiplier, const linear_algebra::DynamicVector<double>&>
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

}  // namespace detail

}  // namespace plato::geometry::extension
