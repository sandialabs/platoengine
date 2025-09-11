#ifndef PLATO_CRITERIA_EXTENSION_OVERHANGCRITERION
#define PLATO_CRITERIA_EXTENSION_OVERHANGCRITERION

#include "plato/analysis/AnalysisDomainMesh.hpp"
#include "plato/criteria/library/CriterionRegistration.hpp"
#include "plato/linear_algebra/DynamicVector.hpp"
#include "plato/third_party_integration/common/Vector3.hpp"
#include "plato/third_party_integration/stk_io/Triangle.hpp"

namespace plato::criteria::extension
{
/// @brief Computes a scalar value representing the amount of overhang wrt a build direction and overhang angle.
struct OverhangCriterion
{
    [[nodiscard]] double f(const analysis::AnalysisDomainMesh& aAnalysisDomainMesh) const;
    [[nodiscard]] linear_algebra::DynamicVector<double> df(
        const analysis::AnalysisDomainMesh& aAnalysisDomainMesh) const;

    static constexpr auto kCriterionName = std::string_view{"overhang"};
};

/// @brief Creates a Function object from a OverhangCriterion
[[nodiscard]] auto make_overhang_function() -> library::CriterionFunction;

namespace detail
{

using namespace plato::third_party_integration::common;
using namespace plato::third_party_integration::stk_io;

[[nodiscard]] double exponential_step_function(const double& aInput);
[[nodiscard]] double smoothing_function(const double& aInput);
[[nodiscard]] double overhang(const double& aAngleDotBuildDirection,
                              const double& aOverhangAngleThreshold,
                              const double& aStepTransitionWidth);
[[nodiscard]] double overhang_from_triangle_node_coordinates(const Coordinate& aNode1,
                                                             const Coordinate& aNode2,
                                                             const Coordinate& aNode3,
                                                             const double& aOverhangAngleThreshold,
                                                             const double& aStepTransitionWidth,
                                                             const Vector3& aBuildDirection);
[[nodiscard]] double area_weighted_overhang_from_triangle(const Triangle& aTriangle,
                                                          const double& aOverhangAngleThreshold,
                                                          const double& aStepTransitionWidth,
                                                          const Vector3& aBuildDirection);
}  // namespace detail

}  // namespace plato::criteria::extension

#endif
