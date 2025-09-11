#include "plato/criteria/extension/OverhangCriterion.hpp"

#include <numeric>
#include <vector>

#include "plato/criteria/library/CriterionRegistration.hpp"
#include "plato/mesh/EntityCounts.hpp"
#include "plato/mesh/EntityRetrieval.hpp"
#include "plato/mesh/Mesh.hpp"
#include "plato/third_party_integration/stk_io/Triangle.hpp"

namespace plato::criteria::extension
{
namespace
{
using Registration =
    library::CriterionRegistration<library::Parallelization::kSerial, library::FunctionDimension::kScalar>;

[[maybe_unused]] static auto kOverhangRegistration =
    Registration{library::builtin_criterion_registration_name(OverhangCriterion::kCriterionName),
                 [](const library::CriterionInput&) { return make_overhang_function(); }};
}  // namespace

double OverhangCriterion::f(const analysis::AnalysisDomainMesh& /*aAnalysisDomainMesh*/) const
{
    /*
    namespace tpi = plato::third_party_integration;

    const auto tMesh = mesh::EntityRetrieval{mesh::Mesh{aAnalysisDomainMesh.mFileName}};
    */
    return 0.0;
}

linear_algebra::DynamicVector<double> OverhangCriterion::df(
    const analysis::AnalysisDomainMesh& aAnalysisDomainMesh) const
{
    const auto tMesh = mesh::EntityCounts{mesh::Mesh{aAnalysisDomainMesh.mFileName}};
    const unsigned int tSpatialDim = tMesh.spatialDimensions();
    const unsigned int tNumberOfNodes = tMesh.numberOfNodes();
    const unsigned int tSize = static_cast<unsigned int>(tSpatialDim * tNumberOfNodes);
    std::vector<double> tCoordinates(tSize, 1);
    return linear_algebra::DynamicVector<double>(std::move(tCoordinates));
}

auto make_overhang_function() -> library::CriterionFunction
{
    return core::make_function_with_first_derivative(
        [](const analysis::AnalysisDomainMesh& mesh) { return OverhangCriterion{}.f(mesh); },
        [](const analysis::AnalysisDomainMesh& mesh) { return OverhangCriterion{}.df(mesh); });
}

namespace detail
{

using namespace plato::third_party_integration::common;
using namespace plato::third_party_integration::stk_io;

[[nodiscard]] double exponential_step_function(const double& aInput)
{
    return aInput > 0.0 ? std::exp(-1.0 / aInput) : 0.0;
}

[[nodiscard]] double smoothing_function(const double& aInput)
{
    return exponential_step_function(aInput) /
           (exponential_step_function(aInput) + exponential_step_function(1.0 - aInput));
}

[[nodiscard]] double overhang(const double& aAngleDotBuildDirection,
                              const double& aOverhangAngleThreshold,
                              const double& aStepTransitionWidth)
{
    // The calculation of the return value will be determined by whether the input value, aAngleDotBuildDirection,
    // is in one of three ranges: 1) less than the step function transition region (return 1.0), 2) inside the step
    // function transiion region (calculate transition value), or 3) to the right of the transition region (return 0.0).
    double tReturnValue = 0.0;
    if (aAngleDotBuildDirection <= (aOverhangAngleThreshold - aStepTransitionWidth))
    {
        tReturnValue = 1.0;
    }
    else if (aAngleDotBuildDirection < aOverhangAngleThreshold)
    {
        tReturnValue = smoothing_function((aOverhangAngleThreshold - aAngleDotBuildDirection) / aStepTransitionWidth);
    }
    return tReturnValue;
}

[[nodiscard]] double overhang_from_triangle_node_coordinates(const Coordinate& aNode1,
                                                             const Coordinate& aNode2,
                                                             const Coordinate& aNode3,
                                                             const double& aOverhangAngleThreshold,
                                                             const double& aStepTransitionWidth,
                                                             const Vector3& aBuildDirection)
{
    const Triangle tTriangle{aNode1, aNode2, aNode3};
    const Vector3 tNormal = tTriangle.normal();
    const double tNormalDotBuildDirection = dot(tNormal, aBuildDirection);
    return overhang(tNormalDotBuildDirection, aOverhangAngleThreshold, aStepTransitionWidth);
}

[[nodiscard]] double area_weighted_overhang_from_triangle(const Triangle& aTriangle,
                                                          const double& aOverhangAngleThreshold,
                                                          const double& aStepTransitionWidth,
                                                          const Vector3& aBuildDirection)
{
    const double tArea = aTriangle.volume();
    return tArea * overhang_from_triangle_node_coordinates(aTriangle.p0, aTriangle.p1, aTriangle.p2,
                                                           aOverhangAngleThreshold, aStepTransitionWidth,
                                                           aBuildDirection);
}

}  // namespace detail

}  // namespace plato::criteria::extension
