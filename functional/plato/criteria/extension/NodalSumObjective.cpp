#include "plato/criteria/extension/NodalSumObjective.hpp"

#include <numeric>
#include <vector>

#include "plato/criteria/library/CriterionRegistration.hpp"
#include "plato/input_parser/InputEnumTypes.hpp"
#include "plato/third_party_integration/stk_io/Utilities.hpp"

namespace plato::criteria::extension
{
namespace
{
[[maybe_unused]] static auto kNodalSumRegistration =
    library::CriterionRegistration{library::builtin_criterion_registration_name(NodalSumObjective::kCriterionName),
                                   [](const library::CriterionInput&) { return make_nodal_sum_function(); }};
}

double NodalSumObjective::f(const core::MeshProxy& aMeshProxy) const
{
    const auto tBulk = third_party_integration::stk_io::read_mesh_bulk_data(aMeshProxy.mFileName);
    assert(tBulk);
    const std::vector<double> tCoordinates = third_party_integration::stk_io::flattened_nodal_coordinates(*tBulk);
    return std::accumulate(tCoordinates.begin(), tCoordinates.end(), 0.0);
}

linear_algebra::DynamicVector<double> NodalSumObjective::df(const core::MeshProxy& aMeshProxy) const
{
    const auto tBulk = third_party_integration::stk_io::read_mesh_bulk_data(aMeshProxy.mFileName);
    assert(tBulk);
    const unsigned int tSpatialDim = third_party_integration::stk_io::spatial_dimensions(*tBulk);
    const unsigned int tNumberOfNodes = third_party_integration::stk_io::node_size(*tBulk);
    const unsigned int tSize = static_cast<unsigned int>(tSpatialDim * tNumberOfNodes);
    std::vector<double> tCoordinates(tSize, 1);
    return linear_algebra::DynamicVector<double>(std::move(tCoordinates));
}

auto make_nodal_sum_function() -> core::Function<double, linear_algebra::DynamicVector<double>, const core::MeshProxy&>
{
    return core::make_function([](const core::MeshProxy& mesh) { return NodalSumObjective{}.f(mesh); },
                               [](const core::MeshProxy& mesh) { return NodalSumObjective{}.df(mesh); });
}
}  // namespace plato::criteria::extension
