#ifndef PLATO_CRITERIA_EXTENSION_OVERHANGCRITERION
#define PLATO_CRITERIA_EXTENSION_OVERHANGCRITERION

#include "plato/analysis/AnalysisDomainMesh.hpp"
#include "plato/criteria/library/CriterionRegistration.hpp"
#include "plato/linear_algebra/DynamicVector.hpp"
#include "plato/third_party_integration/common/Vector3.hpp"
#include "plato/third_party_integration/krino/Utilities.hpp"
#include "plato/third_party_integration/stk_io/Triangle.hpp"

namespace plato::criteria::extension
{
struct ParsedInputParams
{
    double transition_width{0.1};
    double overhang_angle_threshold{-std::sqrt(2.0) / 2.0};
    plato::third_party_integration::common::Vector3 build_direction{0, 0, 1};
};

/// @brief Computes a scalar value representing the amount of overhang wrt a build direction and overhang angle.
struct OverhangCriterion
{
    OverhangCriterion(const ParsedInputParams& aInputParams);
    [[nodiscard]] double f(const analysis::AnalysisDomainMesh& aAnalysisDomainMesh) const;
    [[nodiscard]] linear_algebra::DynamicVector<double> df(
        const analysis::AnalysisDomainMesh& aAnalysisDomainMesh) const;

    double mTransitionWidth;
    plato::third_party_integration::common::Vector3 mBuildDirection;
    double mOverhangAngleThreshold;

    static constexpr auto kCriterionName = std::string_view{"overhang"};
};

/// @brief Creates a Function object from a OverhangCriterion
[[nodiscard]] auto make_overhang_function(const library::CriterionInput& aCriterionInput) -> library::CriterionFunction;

namespace detail
{

using namespace plato::third_party_integration::common;
using namespace plato::third_party_integration::stk_io;
using namespace plato::third_party_integration::krino;

[[nodiscard]] double exponential_step_function(const double& aInput);
[[nodiscard]] double d_exponential_step_function(const double& aInput);
[[nodiscard]] double smoothing_function(const double& aInput);
[[nodiscard]] double d_smoothing_function(const double& aInput);
[[nodiscard]] double overhang(const double& aAngleDotBuildDirection,
                              const double& aOverhangAngleThreshold,
                              const double& aStepTransitionWidth);
[[nodiscard]] double d_overhang(const double& aAngleDotBuildDirection,
                                const double& aOverhangAngleThreshold,
                                const double& aStepTransitionWidth);
[[nodiscard]] double overhang_from_triangle_node_coordinates(const Coordinate& aNode1,
                                                             const Coordinate& aNode2,
                                                             const Coordinate& aNode3,
                                                             const double& aOverhangAngleThreshold,
                                                             const double& aStepTransitionWidth,
                                                             const Vector3& aBuildDirection);
[[nodiscard]] double d_overhang_from_triangle_node_coordinates(const Coordinate& aNode1,
                                                               const Coordinate& aNode2,
                                                               const Coordinate& aNode3,
                                                               const double& aOverhangAngleThreshold,
                                                               const double& aStepTransitionWidth,
                                                               const Vector3& aBuildDirection);
[[nodiscard]] double area_weighted_overhang_from_triangle(const Triangle& aTriangle,
                                                          const double& aOverhangAngleThreshold,
                                                          const double& aStepTransitionWidth,
                                                          const Vector3& aBuildDirection);
[[nodiscard]] auto get_gradient_contribution_for_triangle(const Triangle& aTriangle,
                                                          const double& aOverhangAngleThreshold,
                                                          const double& aStepTransitionWidth,
                                                          const Vector3& aBuildDirection) -> std::vector<double>;
[[nodiscard]] auto parse_input_deck(const std::string& aFilename) -> ParsedInputParams;
}  // namespace detail

}  // namespace plato::criteria::extension

#endif
