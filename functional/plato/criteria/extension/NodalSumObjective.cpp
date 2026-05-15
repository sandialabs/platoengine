#include "plato/criteria/extension/NodalSumObjective.hpp"

#include <numeric>
#include <utility>
#include <vector>

#include "plato/core/Function.hpp"
#include "plato/criteria/library/CriterionRegistration.hpp"
#include "plato/criteria/library/CriterionTraits.hpp"
#include "plato/linear_algebra/DynamicVector.hpp"
#include "plato/mesh/EntityCounts.hpp"
#include "plato/mesh/EntityRetrieval.hpp"
#include "plato/mesh/Mesh.hpp"
#include "plato/services/AppConfiguration.hpp"
#include "plato/third_party_integration/common/Vector3.hpp"

namespace plato::criteria::extension
{
namespace
{
using Registration =
    library::CriterionRegistration<library::Parallelization::kSerial, library::FunctionDimension::kScalar>;

const auto kConfiguration = services::CriterionConfiguration{
    .mName = std::string{NodalSumObjective::kCriterionName}, .mIsParallelized = false, .mIsScalar = true};

[[maybe_unused]] static const auto kNodalSumRegistration = Registration{
    library::builtin_criterion_registration_name(NodalSumObjective::kCriterionName),
    [](const library::CriterionInput&) {
        return library::FunctionWithConfiguration{.mFunction = make_nodal_sum_function(),
                                                  .mConfiguration = kConfiguration};
    }};
}  // namespace

auto NodalSumObjective::f(const analysis::AnalysisDomainMesh& aAnalysisDomainMesh) const -> double
{
    namespace tpi = plato::third_party_integration;

    const auto tMesh = mesh::EntityRetrieval{mesh::Mesh{aAnalysisDomainMesh.mFileName}};
    const auto tCoordinates = tMesh.nodalCoordinates();
    const auto tCoordinateSum = std::accumulate(tCoordinates.begin(), tCoordinates.end(), tpi::common::Coordinate{});
    return tCoordinateSum.x + tCoordinateSum.y + tCoordinateSum.z;
}

auto NodalSumObjective::df(const analysis::AnalysisDomainMesh& aAnalysisDomainMesh) const
    -> linear_algebra::DynamicVector<double>
{
    const auto tMesh = mesh::EntityCounts{mesh::Mesh{aAnalysisDomainMesh.mFileName}};
    const unsigned int tSpatialDim = tMesh.spatialDimensions();
    const unsigned int tNumberOfNodes = tMesh.numberOfNodes();
    const auto tSize = static_cast<unsigned int>(tSpatialDim * tNumberOfNodes);
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
