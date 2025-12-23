#ifndef PLATO_CRITERIA_EXTENSION_OVERHANGCRITERION
#define PLATO_CRITERIA_EXTENSION_OVERHANGCRITERION

#include "plato/analysis/AnalysisDomainMesh.hpp"
#include "plato/criteria/library/CriterionRegistration.hpp"
#include "plato/input_parser/FileList.hpp"
#include "plato/input_parser/Point.hpp"
#include "plato/linear_algebra/DynamicVector.hpp"
#include "plato/third_party_integration/common/Vector3.hpp"
#include "plato/third_party_integration/krino/Utilities.hpp"

///@brief Input struct with parser to read in overhang constraint parameters
// clang-format off
BOOST_FUSION_DEFINE_STRUCT(
(plato)(input_parser),overhang_criterion,
(boost::optional<plato::input_parser::Point>, build_direction)
(boost::optional<double>, overhang_angle_in_degrees)
(boost::optional<double>, transition_width)
(boost::optional<plato::input_parser::FileList>, additional_evaluation_sidesets)
)
// clang-format on

namespace plato::criteria::extension
{

/// @brief Computes a scalar value representing the amount of overhang wrt a build direction and overhang angle.
struct OverhangCriterion
{
    OverhangCriterion(const input_parser::overhang_criterion& aInputParams,
                      const library::CriterionInput& aCriterionInput);
    OverhangCriterion(const double aTransitionWidth,
                      const third_party_integration::common::Vector3& aBuildDirection,
                      const double aOverhangAngleThreshold,
                      const std::vector<std::string>& aEvaluationSidesetNames);
    [[nodiscard]] double f(const analysis::AnalysisDomainMesh& aAnalysisDomainMesh) const;
    [[nodiscard]] linear_algebra::DynamicVector<double> df(
        const analysis::AnalysisDomainMesh& aAnalysisDomainMesh) const;

    double mTransitionWidth;
    third_party_integration::common::Vector3 mBuildDirection;
    double mOverhangAngleThreshold;
    std::vector<std::string> mEvaluationSidesets;
    components::ComponentType mComponentType;
    std::string mName;

    static constexpr auto kCriterionName = std::string_view{"overhang"};
};

/// @brief Creates a Function object from a OverhangCriterion
[[nodiscard]] auto make_overhang_function(const library::CriterionInput& aCriterionInput) -> library::CriterionFunction;

namespace detail
{
// The following 3 functions--exponential_step_function, smoothing_function, and
// overhang_value_from_normal_and_build_direction-- combine to create a smooth function in a finite interval that goes
// from 0 to 1 over a small transition region and then outside that transition region is exactly 0 or 1. This requires
// logic/branching but the way they are defined results in a C infinity function. There is a Wikipedia page that
// describes it: https://en.wikipedia.org/wiki/Bump_function.
/// @brief Exponential function used as a building block for generating a smooth overhang step function going from 0
/// to 1.
[[nodiscard]] double exponential_step_function(const double aInput);
/// @brief Smooth step function for transitioning overhang values from 0 to 1.
[[nodiscard]] double smoothing_function(const double aInput);
/// @brief Calculate the overhang value given the angle of the triangle normal dotted with the build direction @
/// aAngleDotBuildDirection.
[[nodiscard]] double overhang_value_from_normal_and_build_direction(const double aAngleDotBuildDirection,
                                                                    const OverhangCriterion& aOverhangCriterion);
/// @brief Derivattive of exponential function used as a building block for generating a smooth overhang step function
/// going from 0 to 1.
[[nodiscard]] double d_exponential_step_function(const double aInput);
/// @brief Derivative of smooth step function for transitioning overhang values from 0 to 1.
[[nodiscard]] double d_smoothing_function(const double aInput);
/// @brief Calculate the derivative of the overhang value given the angle of the triangle normal dotted with the build
/// direction @ aAngleDotBuildDirection.
[[nodiscard]] double d_overhang_value_from_normal_and_build_direction(const double aAngleDotBuildDirection,
                                                                      const OverhangCriterion& aOverhangCriterion);
/// @brief Calculate the amount of overhang for a single triangle.
[[nodiscard]] double overhang_from_triangle(const third_party_integration::krino::SensitivityTriangle& aTriangle,
                                            const OverhangCriterion& aOverhangCriterion);
/// @brief Calculate the derivative of the amount of overhang for a single triangle.
[[nodiscard]] double d_overhang_from_triangle(const third_party_integration::krino::SensitivityTriangle& aTriangle,
                                              const OverhangCriterion& aOverhangCriterion);
/// @brief Calculate the area-weighted amount of overhang for a single triangle.
[[nodiscard]] double area_weighted_overhang_from_triangle(
    const third_party_integration::krino::SensitivityTriangle& aTriangle, const OverhangCriterion& aOverhangCriterion);
/// @brief Calculate the full overhang gradient vector contribution for the nodes in a single triangle.
[[nodiscard]] auto get_gradient_contribution_for_triangle(
    const third_party_integration::krino::SensitivityTriangle& aTriangle, const OverhangCriterion& aOverhangCriterion)
    -> third_party_integration::krino::TriangleGradient;
/// @brief Parse the overhang parameter input deck.
[[nodiscard]] auto parse_input_deck(const std::string& aFilename) -> input_parser::overhang_criterion;
/// @brief Given a list of triangles @ aTriangles, generate a sparse overhang gradient map for the nodes in the
/// triangles.
[[nodiscard]] auto calculate_gradient_map_from_triangles(
    const std::vector<third_party_integration::krino::SensitivityTriangle>& aTriangles,
    const OverhangCriterion& aOverhangCriterion)
    -> std::unordered_map<third_party_integration::krino::GlobalNodeID, third_party_integration::krino::Sensitivity>;
/// @brief Given a sparse overhang gradient map @ aGradientMap, generate a dense overhang gradient vector for all nodes
/// in the background mesh.
[[nodiscard]] auto get_full_gradient_vector_from_gradient_map(
    const std::unordered_map<third_party_integration::krino::GlobalNodeID, third_party_integration::krino::Sensitivity>&
        aGradientMap,
    const std::vector<size_t>& aAllNodeIds) -> std::vector<double>;
/// @brief Given a list of sideset names @ aEvaluationSidesetNames, generate a list of triangles to calculate overhang
/// on.
[[nodiscard]] auto triangles_to_evaluate_over(const std::string& aMeshFileName,
                                              const std::vector<std::string>& aEvaluationSidesetNames)
    -> std::vector<third_party_integration::krino::SensitivityTriangle>;
/// @brief Function for converting from angle in degrees to overhang threshold value.
[[nodiscard]] double convert_angle_to_threshold_value(const double aAngle);
}  // namespace detail

}  // namespace plato::criteria::extension

#endif
