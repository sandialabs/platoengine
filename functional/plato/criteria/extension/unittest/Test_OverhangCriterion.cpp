#include <gtest/gtest.h>

#include <format>
#include <fstream>
#include <numeric>
#include <ranges>

#include "plato/analysis/AnalysisDomainMesh.hpp"
#include "plato/analysis/Utilities.hpp"
#include "plato/criteria/extension/OverhangCriterion.hpp"
#include "plato/third_party_integration/common/test_utilities/CoordinateTestUtilities.hpp"
#include "plato/third_party_integration/krino/TriangleUtilities.hpp"
#include "plato/third_party_integration/stk_io/WriteUtilities.hpp"
#include "plato/utilities/DataFilePath.hpp"
#include "plato/utilities/Exception.hpp"
#include "plato/utilities/GradientChecker.hpp"
#include "plato/utilities/StringUtilities.hpp"
#include "plato/utilities/Zip.hpp"

namespace plato::criteria::extension::unittest
{

using namespace plato::third_party_integration::common;
using namespace plato::third_party_integration::krino;

constexpr double kTolerance{1e-14};
constexpr Vector3 kBuildDirection{.mX = 0, .mY = 0, .mZ = -1};
constexpr double kStepTransitionWidth{0.05};
const double kOverhangAngleThreshold{-std::sqrt(2.0) / 2.0};
OverhangCriterion kCriterion(kStepTransitionWidth, kBuildDirection, kOverhangAngleThreshold, {});
const auto kBlock1 = analysis::AnalysisDomainMesh::ScalarFieldVector{{0, 0, 0.0}, {1, 1, 1.0}, {2, 2, 2.0}};
constexpr auto kBlock1ID = 1U;

TEST(OverhangCriterion, ExponentialStepFunction)
{
    const std::vector<double> tInputs{-1.0, 0.0, .26, .74, 1.0, 100.0};
    const std::vector<double> tExpectedValues{
        0.0, 0.0, 0.021361739175007062, 0.25889017256861729, 0.367879441171442, 0.990049833749168};
    for (const auto& [tCurInput, tCurExpectedValue] : utilities::Zip{tInputs, tExpectedValues})
    {
        EXPECT_NEAR(detail::exponential_step_function(tCurInput), tCurExpectedValue, kTolerance);
    }
}

TEST(OverhangCriterion, SmoothingFunction)
{
    const std::vector<double> tInputs{-1.0, 0.0, 1.0, 100.0, .26, .74};
    const std::vector<double> tExpectedValues{0.0, 0.0, 1.0, 1.0, 0.076223348636953722, 0.92377665136304632};
    for (const auto& [tCurInput, tCurExpectedValue] : utilities::Zip{tInputs, tExpectedValues})
    {
        EXPECT_NEAR(detail::smoothing_function(tCurInput), tCurExpectedValue, kTolerance);
    }
}

TEST(OverhangCriterion, Overhang)
{
    const std::vector<double> tNormalDotBuildDirectionValues{
        -5.0, -.8, kOverhangAngleThreshold - (kStepTransitionWidth / 2.0), -.69, 100.0};
    const std::vector<double> tExpectedValues{1.0, 1.0, 0.5, 0.0, 0.0};
    for (const auto& [tCurNormalDotBuildDirectionValue, tCurExpectedValue] :
         utilities::Zip{tNormalDotBuildDirectionValues, tExpectedValues})
    {
        EXPECT_NEAR(
            detail::overhang_value_from_normal_and_build_direction(tCurNormalDotBuildDirectionValue, kCriterion),
            tCurExpectedValue, kTolerance);
    }
}

TEST(OverhangCriterion, OverhangFromTriangleCoords)
{
    constexpr Coordinate tNode1{.mX = 0, .mY = 0, .mZ = 0};
    constexpr Coordinate tNode2{.mX = 1, .mY = 0, .mZ = 0};
    const std::vector<Coordinate> tThirdNodeOptions{{.mX = 1, .mY = 1, .mZ = 0},
                                                    {.mX = 1, .mY = 1, .mZ = -1},
                                                    {.mX = 1, .mY = .732106781186548, .mZ = -.681189886111555},
                                                    {.mX = 1, .mY = 1, .mZ = -.8}};
    const std::vector<double> tExpectedValues{1.0, 0.0, 0.5, 1.0};
    for (const auto& [tCurNode3Option, tCurExpectedValue] : utilities::Zip{tThirdNodeOptions, tExpectedValues})
    {
        SensitivityTriangle tTriangle{NodeIDCoordsPair{23, tNode1}, NodeIDCoordsPair{25, tNode2},
                                      NodeIDCoordsPair{26, tCurNode3Option}};
        EXPECT_NEAR(detail::overhang_from_triangle(tTriangle, kCriterion), tCurExpectedValue, kTolerance);
    }
}

TEST(OverhangCriterion, OverhangFromTriangle)
{
    constexpr SensitivityTriangle tTriangle{NodeIDCoordsPair{5, {.mX = 0, .mY = 0, .mZ = 0}},
                                            NodeIDCoordsPair{6, {.mX = 1, .mY = 0, .mZ = 0}},
                                            NodeIDCoordsPair{7, {.mX = 1, .mY = 1, .mZ = 0}}};
    constexpr double tExpectedValue{0.5};
    EXPECT_NEAR(detail::area_weighted_overhang_from_triangle(tTriangle, kCriterion), tExpectedValue, kTolerance);
}

namespace
{
void check_first_order_truncation_error(const auto& aF,
                                        const auto& aDF,
                                        const plato::utilities::GradientCheckParameters& aGradCheckParams,
                                        const double aX,
                                        const double aDirection,
                                        const double aFirstOrderTruncationTolerance,
                                        const double aLastFiniteDifferenceError)
{
    const auto tChecker = plato::utilities::GradientChecker{aF, aDF};

    EXPECT_NEAR(tChecker.maxFirstOrderTruncationError(aX, aDirection, aGradCheckParams), 0.0,
                aFirstOrderTruncationTolerance)
        << tChecker.table(aX, aDirection, aGradCheckParams);
    const auto tErrors = tChecker.finiteDifferenceErrors(aX, aDirection, aGradCheckParams);
    ASSERT_FALSE(tErrors.empty());
    EXPECT_NEAR(tErrors.back().mValue, 0.0, aLastFiniteDifferenceError)
        << "Full gradient check:\n"
        << tChecker.table(aX, aDirection, aGradCheckParams);
}
}  // namespace

TEST(OverhangCriterion, dExponentialStepFunction)
{
    constexpr plato::utilities::GradientCheckParameters tGradCheckParams{
        .mStepDelta = .1, .mNumSteps = 6, .mInitialStepSize = .01};
    constexpr auto tFirstOrderTruncationTolerance{1e-1};
    constexpr auto tLastFiniteDifferenceError{1e-6};
    constexpr auto tX = 0.26;
    constexpr auto tDirection = 1.0;
    check_first_order_truncation_error([](const double x) { return detail::exponential_step_function(x); },
                                       [](const double x) { return detail::d_exponential_step_function(x); },
                                       tGradCheckParams, tX, tDirection, tFirstOrderTruncationTolerance,
                                       tLastFiniteDifferenceError);
}

TEST(OverhangCriterion, dSmoothingFunction)
{
    constexpr plato::utilities::GradientCheckParameters tGradCheckParams{
        .mStepDelta = .1, .mNumSteps = 6, .mInitialStepSize = .01};
    constexpr auto tFirstOrderTruncationTolerance{1e-1};
    constexpr auto tLastFiniteDifferenceError{1e-6};
    constexpr auto tX = 0.74;
    constexpr auto tDirection = 1.0;
    check_first_order_truncation_error([](const double x) { return detail::smoothing_function(x); },
                                       [](const double x) { return detail::d_smoothing_function(x); }, tGradCheckParams,
                                       tX, tDirection, tFirstOrderTruncationTolerance, tLastFiniteDifferenceError);
}

TEST(OverhangCriterion, dOverhang)
{
    constexpr plato::utilities::GradientCheckParameters tGradCheckParams{
        .mStepDelta = .5, .mNumSteps = 6, .mInitialStepSize = .0001};
    constexpr auto tFirstOrderTruncationTolerance{6e-1};
    constexpr auto tLastFiniteDifferenceError{1e-8};
    const auto tX = kOverhangAngleThreshold - (kStepTransitionWidth / 2.0);
    constexpr auto tDirection = 1.0;
    check_first_order_truncation_error(
        [](const double aNormalDotBuildDirection)
        { return detail::overhang_value_from_normal_and_build_direction(aNormalDotBuildDirection, kCriterion); },
        [](const double aNormalDotBuildDirection)
        { return detail::d_overhang_value_from_normal_and_build_direction(aNormalDotBuildDirection, kCriterion); },
        tGradCheckParams, tX, tDirection, tFirstOrderTruncationTolerance, tLastFiniteDifferenceError);
}

namespace
{
[[nodiscard]] SensitivityTriangle build_single_tri_from_coords(const std::vector<double>& aCoords)
{
    const Coordinate tNode1{.mX = aCoords[0], .mY = aCoords[1], .mZ = aCoords[2]};
    const Coordinate tNode2{.mX = aCoords[3], .mY = aCoords[4], .mZ = aCoords[5]};
    const Coordinate tNode3{.mX = aCoords[6], .mY = aCoords[7], .mZ = aCoords[8]};
    return SensitivityTriangle{NodeIDCoordsPair{1, tNode1}, NodeIDCoordsPair{2, tNode2}, NodeIDCoordsPair{3, tNode3}};
}
}  // namespace

TEST(OverhangCriterion, SingleTriangleDerivative)
{
    constexpr auto tAbsoluteError = 7e-3;
    const double tOverhangThreshold = -std::sqrt(2.0) / 2.0;
    constexpr double tTransitionWidth = 0.05;
    constexpr Vector3 tBuildDirection = {.mX = 0, .mY = 0, .mZ = 1};
    const OverhangCriterion tCriterion(tTransitionWidth, tBuildDirection, tOverhangThreshold, {});
    const auto tChecker = plato::utilities::GradientChecker{
        [tCriterion](const linear_algebra::DynamicVector<double>& aTriNodalCoords)
        {
            const auto tTriangle = build_single_tri_from_coords(aTriNodalCoords.stdVector());
            return detail::area_weighted_overhang_from_triangle(tTriangle, tCriterion);
        },
        [tCriterion](const linear_algebra::DynamicVector<double>& aTriNodalCoords,
                     const linear_algebra::DynamicVector<double>& aDirection)
        {
            const auto tTriangle = build_single_tri_from_coords(aTriNodalCoords.stdVector());
            const auto tSensitivities = detail::get_gradient_contribution_for_triangle(tTriangle, tCriterion);
            std::vector<double> tGradientValues;
            for (const auto& tCurSensitivity : tSensitivities)
            {
                tGradientValues.push_back(tCurSensitivity.second.mX);
                tGradientValues.push_back(tCurSensitivity.second.mY);
                tGradientValues.push_back(tCurSensitivity.second.mZ);
            }
            const auto tGradient = linear_algebra::DynamicVector<double>{tGradientValues};
            return tGradient.dot(aDirection);
        }};
    const auto tGradientCheckParameters =
        plato::utilities::GradientCheckParameters{.mStepDelta = 0.5, .mNumSteps = 10, .mInitialStepSize = .001};
    const auto tNodalCoordinates = linear_algebra::DynamicVector<double>{0, 0, 0, 0, 1, -.1, 1, 0, -.1};
    const auto tDirection = linear_algebra::DynamicVector<double>{.10, -.10, 0.05, 0.03, -0.09, 0.2, -.04, -.3, .07};

    EXPECT_NEAR(tChecker.maxFirstOrderTruncationError(tNodalCoordinates, tDirection, tGradientCheckParameters), 0.0,
                tAbsoluteError)
        << tChecker.table(tNodalCoordinates, tDirection, tGradientCheckParameters);
}

TEST(OverhangCriterion, GradientMapFromMulitpleTriangles)
{
    // clang-format off

    /*                     6  (1,2,0)         */
    /*                    /\                  */
    /*                   /  \                 */
    /*                  /    \                */
    /*       (.5,1,0) 4 ------ 5 (1.5,1,0)    */   
    /*                / \    / \              */
    /*               /   \  /   \             */
    /*              /     \/     \            */
    /*    (0,0,0) 1-------2------ 3 (2,0,0)   */    
    /*                   (1,0,0)              */
    //
    //
    // 4 triangles as shown above with nodes {1,2,4}, {2,3,5}, {2,5,4}, {4,5,6}

    // clang-format on

    const NodeIDCoordsPair tNode1{1, Coordinate{.mX = 0., .mY = 0., .mZ = 0.}};
    const NodeIDCoordsPair tNode2{2, Coordinate{.mX = 1., .mY = 0., .mZ = 0.}};
    const NodeIDCoordsPair tNode3{3, Coordinate{.mX = 2., .mY = 0., .mZ = 0.}};
    const NodeIDCoordsPair tNode4{4, Coordinate{.mX = .5, .mY = 1., .mZ = 0.}};
    const NodeIDCoordsPair tNode5{5, Coordinate{.mX = 1.5, .mY = 1., .mZ = 0.}};
    const NodeIDCoordsPair tNode6{6, Coordinate{.mX = 1., .mY = 2., .mZ = 0.}};
    const std::vector<SensitivityTriangle> tTriangles{
        {tNode1, tNode2, tNode4}, {tNode2, tNode3, tNode5}, {tNode2, tNode5, tNode4}, {tNode4, tNode5, tNode6}};
    const double tOverhangThreshold = -std::sqrt(2.0) / 2.0;
    constexpr double tTransitionWidth = 0.05;
    constexpr Vector3 tBuildDirection = {.mX = 0, .mY = .681189886111555, .mZ = -.732106781186548};
    const OverhangCriterion tCriterion(tTransitionWidth, tBuildDirection, tOverhangThreshold, {});
    constexpr size_t tNumNodes{6};
    constexpr size_t tNumTris{4};

    // Calculate indivdual triangle maps--one for each tri
    std::vector<std::unordered_map<GlobalNodeID, Sensitivity>> tIndividualGradientMaps;
    for (const auto tTriIndex : std::views::iota(0u, tNumTris))
    {
        tIndividualGradientMaps.push_back(
            detail::calculate_gradient_map_from_triangles({tTriangles[tTriIndex]}, tCriterion));
    }
    // Calculate combined triangle map from multiple tris
    const std::unordered_map<GlobalNodeID, Sensitivity> tCombinedGradientMap =
        detail::calculate_gradient_map_from_triangles(tTriangles, tCriterion);

    // Compare results
    for (const auto tNodeIndex : std::views::iota(0UL, tNumNodes))
    {
        constexpr auto tTriangleRange = std::views::iota(0UL, tNumTris);
        const Sensitivity tCurNodeGradient = std::accumulate(
            tTriangleRange.begin(), tTriangleRange.end(), Sensitivity{0., 0., 0.},
            [tIndividualGradientMaps, tNodeIndex](const Sensitivity aCurNodeGradient, const auto aTriIndex)
            {
                return tIndividualGradientMaps[aTriIndex].count(tNodeIndex + 1)
                           ? aCurNodeGradient + tIndividualGradientMaps[aTriIndex].at(tNodeIndex + 1)
                           : aCurNodeGradient;
            });

        // Compare against combined map
        third_party_integration::common::test_utilities::test_double_equality_of_components(
            tCurNodeGradient, tCombinedGradientMap.at(tNodeIndex + 1), TEST_CONTEXT("Checking vector components"));
    }
}

using NodeGradient = std::pair<GlobalNodeID, Sensitivity>;
using TriangleGradient = std::array<NodeGradient, 3>;

TEST(OverhangCriterion, FullGradientVectorFromPartialGradientMap)
{
    const std::unordered_map<GlobalNodeID, Sensitivity> tPartialGradientMap = {{3, {.mX = .1, .mY = .2, .mZ = .3}},
                                                                               {5, {.mX = .9, .mY = -.1, .mZ = -.2}},
                                                                               {9, {.mX = -1., .mY = -2., .mZ = -3.}}};
    const std::vector<size_t> tAllNodeIDs = {2, 3, 5, 6, 7, 9, 11, 12};
    const std::vector<double> tFullGradientVector =
        detail::get_full_gradient_vector_from_gradient_map(tPartialGradientMap, tAllNodeIDs);
    const std::vector<double> tGoldValues = {0, 0, 0, .1,  .2,  .3,  .9, -.1, -.2, 0, 0, 0,
                                             0, 0, 0, -1., -2., -3., 0,  0,   0,   0, 0, 0};
    EXPECT_EQ(tFullGradientVector, tGoldValues);
}

namespace
{

void create_valid_overhang_input_file(const std::filesystem::path& aFilename,
                                      const std::vector<std::string>& aAdditionalSidesetNames)
{
    std::ofstream tTextFile(aFilename);
    tTextFile << "begin overhang\n";
    tTextFile << "build_direction (0, 0, 1)\n";
    tTextFile << "overhang_angle_in_degrees 45\n";
    tTextFile << "transition_width 0.1\n";
    if (!aAdditionalSidesetNames.empty())
    {
        tTextFile << "additional_evaluation_sidesets "
                  << utilities::concatenate_container(aAdditionalSidesetNames, ", ") << std::endl;
    }
    tTextFile << "end\n";
    tTextFile.close();
}

void create_invalid_overhang_input_file(const std::filesystem::path& aFilename)
{
    std::ofstream tTextFile(aFilename);
    tTextFile << "begin overhang\n";
    tTextFile << "build_direction (0, 0)\n";
    tTextFile << "end\n";
    tTextFile.close();
}

OverhangCriterion create_overhang_criterion()
{
    const std::vector<std::string> tSidesetNames{{"surface__void"}};
    constexpr double tTransitionWidth{0.05};
    constexpr double tOverhangAngleInDegrees{45.0};
    const double tOverhangThreshold{detail::convert_angle_to_threshold_value(tOverhangAngleInDegrees)};
    const third_party_integration::common::Vector3 tBuildDirection{.mX = 0, .mY = 1, .mZ = 0};
    return OverhangCriterion(tTransitionWidth, tBuildDirection, tOverhangThreshold, tSidesetNames);
}

}  // namespace

TEST(OverhangCriterion, ParseInputDeck_Correct)
{
    const std::string tFilename{"temp_input_deck.txt"};
    const std::vector<std::string> tSidesets{"my_sideset", "your_sideset"};
    create_valid_overhang_input_file(tFilename, tSidesets);
    const auto& tParams = detail::parse_input_deck(tFilename);
    constexpr auto tAbsoluteError = 1e-10;
    ASSERT_TRUE(tParams.build_direction.has_value());
    ASSERT_TRUE(tParams.overhang_angle_in_degrees.has_value());
    ASSERT_TRUE(tParams.transition_width.has_value());
    ASSERT_TRUE(tParams.additional_evaluation_sidesets.has_value());
    EXPECT_NEAR(tParams.build_direction.value().mX, 0.0, tAbsoluteError);
    EXPECT_NEAR(tParams.build_direction.value().mY, 0.0, tAbsoluteError);
    EXPECT_NEAR(tParams.build_direction.value().mZ, 1.0, tAbsoluteError);
    EXPECT_NEAR(tParams.overhang_angle_in_degrees.value(), 45, tAbsoluteError);
    EXPECT_NEAR(tParams.transition_width.value(), 0.1, tAbsoluteError);
    EXPECT_EQ(tParams.additional_evaluation_sidesets.value().list().mList, tSidesets);
    std::filesystem::remove(tFilename);
}

TEST(OverhangCriterion, ParseInputDeck_Correct_NoAdditionalEvaluationSidesets)
{
    const std::string tFilename{"temp_input_deck.txt"};
    create_valid_overhang_input_file(tFilename, {});
    const auto& tParams = detail::parse_input_deck(tFilename);
    EXPECT_FALSE(tParams.additional_evaluation_sidesets.has_value());
    std::filesystem::remove(tFilename);
}

TEST(OverhangCriterion, ParseInputDeck_InCorrectInputFile)
{
    const std::string tFilename{"temp_input_deck.txt"};
    create_invalid_overhang_input_file(tFilename);
    EXPECT_THROW([[maybe_unused]] const auto& tParams = detail::parse_input_deck(tFilename),
                 plato::utilities::Exception);
    std::filesystem::remove(tFilename);
}

TEST(OverhangCriterion, ParseInputDeck_NonExistentInputFile)
{
    const std::string tFilename{"temp_input_deck.txt"};
    EXPECT_THROW([[maybe_unused]] const auto& tParams = detail::parse_input_deck(tFilename),
                 plato::utilities::Exception);
}

TEST(OverhangCriterion, ToSidesetList_NoAdditionalSidesetNames)
{
    const auto& tSidesetList = detail::to_sideset_list({});
    EXPECT_EQ(tSidesetList.size(), 1);
    EXPECT_STREQ(tSidesetList[0].c_str(), third_party_integration::krino::interface_sideset_name().c_str());
}

TEST(OverhangCriterion, ToSidesetList_AdditionalSidesetNames)
{
    input_parser::FileList tAdditionalSidesetNames{{"cat", "dog"}};
    const auto& tSidesetList = detail::to_sideset_list(tAdditionalSidesetNames);
    EXPECT_EQ(tSidesetList.size(), 3);
    EXPECT_STREQ(tSidesetList[0].c_str(), third_party_integration::krino::interface_sideset_name().c_str());
    EXPECT_STREQ(tSidesetList[1].c_str(), "cat");
    EXPECT_STREQ(tSidesetList[2].c_str(), "dog");
}

TEST(OverhangCriterion, BuildDirection_InputProvided)
{
    input_parser::Point tInputDirection{2, 3, 4};
    const auto& tBuildDirection = detail::build_direction(tInputDirection);
    EXPECT_DOUBLE_EQ(tBuildDirection.mX, 2);
    EXPECT_DOUBLE_EQ(tBuildDirection.mY, 3);
    EXPECT_DOUBLE_EQ(tBuildDirection.mZ, 4);
}

TEST(OverhangCriterion, BuildDirection_NoInputProvided)
{
    const auto& tBuildDirection = detail::build_direction({});
    EXPECT_DOUBLE_EQ(tBuildDirection.mX, 0);
    EXPECT_DOUBLE_EQ(tBuildDirection.mY, 0);
    EXPECT_DOUBLE_EQ(tBuildDirection.mZ, 1);
}

TEST(OverhangCriterion, ConstructionFromEmptyInput)
{
    constexpr auto tNumDefaultEvaluationSidesets{1}; /* "surface__void" is always there by default */
    constexpr auto tDefaultTransitionWidth{0.1};
    constexpr auto tDefaultAngle{45.0};
    const third_party_integration::common::Vector3 tDefaultBuildDirection{0, 0, 1};
    const auto tOverhangInput = input_parser::overhang_criterion{
        /*build_direction*/ boost::none, /*overhang_angle_in_degrees*/ boost::none, /*transition_width*/ boost::none,
        /*additional_evaluation_sidesets*/ boost::none};
    const library::CriterionInput tCriterionInput{};
    const OverhangCriterion tOverhangCriterion{tOverhangInput, tCriterionInput};
    EXPECT_EQ(tOverhangCriterion.mOverhangAngleThreshold, detail::convert_angle_to_threshold_value(tDefaultAngle));
    third_party_integration::common::test_utilities::test_double_equality_of_components(
        tOverhangCriterion.mBuildDirection, tDefaultBuildDirection, TEST_CONTEXT("Checking vector components"));
    EXPECT_EQ(tOverhangCriterion.mTransitionWidth, tDefaultTransitionWidth);
    EXPECT_EQ(tOverhangCriterion.mEvaluationSidesets.size(), tNumDefaultEvaluationSidesets);
}

TEST(OverhangCriterion, ConvertFromAngleToThresholdValue)
{
    constexpr auto tAbsoluteError = 1e-10;
    constexpr auto tAngle{60.0};
    constexpr auto tGold{-0.5};
    const auto tConvertedValue{detail::convert_angle_to_threshold_value(tAngle)};
    EXPECT_NEAR(tConvertedValue, tGold, tAbsoluteError);
}

TEST(OverhangCriterion, TrainglesToEvaluateOver)
{
    constexpr double tTolerance{1e-14};
    constexpr size_t tGoldNumTris{2};
    const third_party_integration::common::Vector3 tGoldNormal{.mX = 0, .mY = 0, .mZ = 1};
    const auto tMeshPath = plato::utilities::data_file_path("two_block_one_interior_sideset_tet.cdf");
    const std::vector<std::string> tSidesetNames{{"my_ss"}};
    const std::vector<SensitivityTriangle> tSensitivityTriangles =
        detail::triangles_to_evaluate_over(tMeshPath.value(), tSidesetNames);
    EXPECT_EQ(tSensitivityTriangles.size(), tGoldNumTris);
    for (const auto tCurTriIndex : std::views::iota(0u, tGoldNumTris))
    {
        const third_party_integration::common::Vector3 tNormal = tSensitivityTriangles[tCurTriIndex].normal();
        EXPECT_NEAR(tNormal.mX, tGoldNormal.mX, tTolerance);
        EXPECT_NEAR(tNormal.mY, tGoldNormal.mY, tTolerance);
        EXPECT_NEAR(tNormal.mZ, tGoldNormal.mZ, tTolerance);
    }
}

TEST(OverhangCriterion, Value)
{
    constexpr double tExpectedValue{0.5};
    const auto& tCriterion = create_overhang_criterion();
    const auto tMeshPath = plato::utilities::data_file_path("one_tet_with_krino_sideset.cdf");
    EXPECT_DOUBLE_EQ(tCriterion.f(analysis::AnalysisDomainMesh{tMeshPath.value(), {}}), tExpectedValue);
}

TEST(OverhangCriterion, Gradient)
{
    const std::vector<double> tExpectedGradient{0.5, 0.0, 0.0, -0.5, 0.0, -0.5, 0.0, 0.0, 0.5, 0.0, 0.0, 0.0};
    const auto& tCriterion = create_overhang_criterion();
    const auto tMeshPath = plato::utilities::data_file_path("one_tet_with_krino_sideset.cdf");
    const linear_algebra::DynamicVector<double> tGradient =
        tCriterion.df(analysis::AnalysisDomainMesh{tMeshPath.value(), {}});
    for (const auto& [tGradientComponent, tExpectedGradientComponent] :
         utilities::Zip{tGradient.stdVector(), tExpectedGradient})
    {
        EXPECT_DOUBLE_EQ(tGradientComponent, tExpectedGradientComponent);
    }
}

TEST(OverhangCriterion, Value_WhenFieldPresent)
{
    const auto& tCriterion = create_overhang_criterion();
    auto tAnalysisDomainMesh =
        analysis::AnalysisDomainMesh{"fake-mesh", analysis::AnalysisDomainMesh::BlockScalarField{{kBlock1ID, kBlock1}}};
    tAnalysisDomainMesh = zero_scalar_field(std::move(tAnalysisDomainMesh));
    EXPECT_THROW([[maybe_unused]] const auto tValue = tCriterion.f(tAnalysisDomainMesh), plato::utilities::Exception);
}

TEST(OverhangCriterion, Gradient_WhenFieldPresent)
{
    const auto& tCriterion = create_overhang_criterion();
    auto tAnalysisDomainMesh =
        analysis::AnalysisDomainMesh{"fake-mesh", analysis::AnalysisDomainMesh::BlockScalarField{{kBlock1ID, kBlock1}}};
    tAnalysisDomainMesh = zero_scalar_field(std::move(tAnalysisDomainMesh));
    EXPECT_THROW([[maybe_unused]] const auto tGradient = tCriterion.df(tAnalysisDomainMesh),
                 plato::utilities::Exception);
}

}  // namespace plato::criteria::extension::unittest
