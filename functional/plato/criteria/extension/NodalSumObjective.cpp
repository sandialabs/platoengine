#include "plato/criteria/extension/NodalSumObjective.hpp"

#include <numeric>
#include <vector>

#include "plato/criteria/library/CriterionRegistration.hpp"
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

double NodalSumObjective::f(const analysis::AnalysisDomainMesh& aAnalysisDomainMesh) const
{
    namespace tpi = plato::third_party_integration;

    const auto tMesh = mesh::EntityRetrieval{mesh::Mesh{aAnalysisDomainMesh.mFileName}};
    const auto tCoordinates = tMesh.nodalCoordinates();
    const auto tCoordinateSum = std::accumulate(tCoordinates.begin(), tCoordinates.end(), tpi::common::Coordinate{});
    return tCoordinateSum.x + tCoordinateSum.y + tCoordinateSum.z;
}

linear_algebra::DynamicVector<double> NodalSumObjective::df(
    const analysis::AnalysisDomainMesh& aAnalysisDomainMesh) const
{
    const auto tMesh = mesh::EntityCounts{mesh::Mesh{aAnalysisDomainMesh.mFileName}};
    const unsigned int tSpatialDim = tMesh.spatialDimensions();
    const unsigned int tNumberOfNodes = tMesh.numberOfNodes();
    const unsigned int tSize = static_cast<unsigned int>(tSpatialDim * tNumberOfNodes);
    std::vector<double> tCoordinates(tSize, 1);
    return linear_algebra::DynamicVector<double>(std::move(tCoordinates));
}

auto make_nodal_sum_function() -> library::CriterionFunction
{
    return core::make_function_with_first_derivative(
        [](const analysis::AnalysisDomainMesh& mesh) { return NodalSumObjective{}.f(mesh); },
        [](const analysis::AnalysisDomainMesh& mesh) { return NodalSumObjective{}.df(mesh); });
}
}  // namespace plato::criteria::extension
