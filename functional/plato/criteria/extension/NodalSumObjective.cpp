#include "plato/criteria/extension/NodalSumObjective.hpp"

#include <numeric>
#include <vector>

#include "plato/criteria/library/CriterionRegistration.hpp"
#include "plato/input_parser/InputEnumTypes.hpp"
#include "plato/utilities/STKUtilities.hpp"

namespace plato::criteria::extension
{
namespace
{
[[maybe_unused]] static auto kNodalSumRegistration = library::CriterionRegistration{
    input_parser::kCodeOptionsTable.toString(input_parser::CodeOptions::kNodalSum).value(),
    [](const library::CriterionInput&) { return make_nodal_sum_function(); }};
}

double NodalSumObjective::f(const core::MeshProxy& aMeshProxy) const
{
    auto tBulk = plato::utilities::read_mesh_bulk_data(aMeshProxy.mFileName.string());
    const std::vector<double> tCoordinates = plato::utilities::nodal_coordinates(tBulk);
    return std::accumulate(tCoordinates.begin(), tCoordinates.end(), 0.0);
}

linear_algebra::DynamicVector<double> NodalSumObjective::df(const core::MeshProxy& aMeshProxy) const
{
    auto tBulk = plato::utilities::read_mesh_bulk_data(aMeshProxy.mFileName.string());
    std::vector<double> tCoordinates = plato::utilities::nodal_coordinates(tBulk);
    std::fill(tCoordinates.begin(), tCoordinates.end(), 1.0);
    return linear_algebra::DynamicVector<double>(std::move(tCoordinates));
}

auto make_nodal_sum_function() -> core::Function<double, linear_algebra::DynamicVector<double>, const core::MeshProxy&>
{
    return core::make_function([](const core::MeshProxy& mesh) { return NodalSumObjective{}.f(mesh); },
                               [](const core::MeshProxy& mesh) { return NodalSumObjective{}.df(mesh); });
}
}  // namespace plato::criteria::extension
