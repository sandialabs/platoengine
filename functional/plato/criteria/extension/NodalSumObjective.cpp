#include "plato/criteria/extension/NodalSumObjective.hpp"

#include <numeric>
#include <vector>

#include "plato/criteria/library/CriterionRegistration.hpp"
#include "plato/input_parser/InputEnumTypes.hpp"
#include "plato/mesh/EntityCounts.hpp"
#include "plato/mesh/EntityRetrieval.hpp"
#include "plato/mesh/Mesh.hpp"

namespace plato::criteria::extension
{
namespace
{
[[maybe_unused]] static auto kNodalSumRegistration =
    library::CriterionRegistration{library::builtin_criterion_registration_name(NodalSumObjective::kCriterionName),
                                   [](const library::CriterionInput&) { return make_nodal_sum_function(); }};
}

double NodalSumObjective::f(const mesh::MeshProxy& aMeshProxy) const
{
    const auto tMesh = mesh::EntityRetrieval{mesh::Mesh{aMeshProxy.mFileName}};
    const std::vector<double> tCoordinates = tMesh.flattenedNodalCoordinates();
    return std::accumulate(tCoordinates.begin(), tCoordinates.end(), 0.0);
}

linear_algebra::DynamicVector<double> NodalSumObjective::df(const mesh::MeshProxy& aMeshProxy) const
{
    const auto tMesh = mesh::EntityCounts{mesh::Mesh{aMeshProxy.mFileName}};
    const unsigned int tSpatialDim = tMesh.spatialDimensions();
    const unsigned int tNumberOfNodes = tMesh.numberOfNodes();
    const unsigned int tSize = static_cast<unsigned int>(tSpatialDim * tNumberOfNodes);
    std::vector<double> tCoordinates(tSize, 1);
    return linear_algebra::DynamicVector<double>(std::move(tCoordinates));
}

auto make_nodal_sum_function() -> core::Function<double, linear_algebra::DynamicVector<double>, const mesh::MeshProxy&>
{
    return core::make_function([](const mesh::MeshProxy& mesh) { return NodalSumObjective{}.f(mesh); },
                               [](const mesh::MeshProxy& mesh) { return NodalSumObjective{}.df(mesh); });
}
}  // namespace plato::criteria::extension
