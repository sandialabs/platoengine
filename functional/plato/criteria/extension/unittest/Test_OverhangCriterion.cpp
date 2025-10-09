#include <gtest/gtest.h>

#include <ios>

#include "plato/criteria/extension/OverhangCriterion.hpp"
#include "plato/test_utilities/GradientChecker.hpp"
#include "plato/utilities/Exception.hpp"
#include "plato/utilities/Zip.hpp"

namespace plato::criteria::extension::unittest
{

using namespace plato::third_party_integration::common;
using namespace plato::third_party_integration::stk_io;

constexpr double kTolerance{1e-14};
constexpr Vector3 kBuildDirection{0, 0, -1};
constexpr double kStepTransitionWidth{0.05};
constexpr double kOverhangAngleThreshold{-std::sqrt(2.0) / 2.0};

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
        EXPECT_NEAR(detail::overhang(tCurNormalDotBuildDirectionValue, kOverhangAngleThreshold, kStepTransitionWidth),
                    tCurExpectedValue, kTolerance);
    }
}

TEST(OverhangCriterion, OverhangFromTriangleCoords)
{
    constexpr Coordinate tNode1{0, 0, 0};
    constexpr Coordinate tNode2{1, 0, 0};
    const std::vector<Coordinate> tThirdNodeOptions{
        {1, 1, 0}, {1, 1, -1}, {1, .732106781186548, -.681189886111555}, {1, 1, -.8}};
    const std::vector<double> tExpectedValues{1.0, 0.0, 0.5, 1.0};
    for (const auto& [tCurNode3Option, tCurExpectedValue] : utilities::Zip{tThirdNodeOptions, tExpectedValues})
    {
        EXPECT_NEAR(
            detail::overhang_from_triangle_node_coordinates(tNode1, tNode2, tCurNode3Option, kOverhangAngleThreshold,
                                                            kStepTransitionWidth, kBuildDirection),
            tCurExpectedValue, kTolerance);
    }
}

TEST(OverhangCriterion, OverhangFromTriangle)
{
    constexpr Triangle tTriangle{{0, 0, 0}, {1, 0, 0}, {1, 1, 0}};
    constexpr double tExpectedValue{0.5};
    EXPECT_NEAR(detail::area_weighted_overhang_from_triangle(tTriangle, kOverhangAngleThreshold, kStepTransitionWidth,
                                                             kBuildDirection),
                tExpectedValue, kTolerance);
}

TEST(OverhangCriterion, dExponentialStepFunction)
{
    constexpr test_utilities::GradientCheckParameters tGradCheckParams{.1, 6, .01};
    constexpr auto tFirstOrderTruncationTolerance{1e-1};
    constexpr auto tLastFiniteDifferenceError{1e-6};
    const auto tChecker =
        plato::test_utilities::GradientChecker{[](const double x) { return detail::exponential_step_function(x); },
                                               [](const double x) { return detail::d_exponential_step_function(x); }};
    constexpr auto tX = 0.26;
    constexpr auto tDirection = 1.0;

    EXPECT_NEAR(tChecker.maxFirstOrderTruncationError(tX, tDirection, tGradCheckParams), 0.0,
                tFirstOrderTruncationTolerance);

    const auto tErrors = tChecker.finiteDifferenceErrors(tX, tDirection, tGradCheckParams);
    ASSERT_FALSE(tErrors.empty());
    EXPECT_NEAR(tErrors.back().mValue, 0.0, tLastFiniteDifferenceError)
        << "Full gradient check:\n"
        << tChecker.table(tX, tDirection, tGradCheckParams);
}

TEST(OverhangCriterion, dSmoothingFunction)
{
    constexpr test_utilities::GradientCheckParameters tGradCheckParams{.1, 6, .01};
    constexpr auto tFirstOrderTruncationTolerance{1e-1};
    constexpr auto tLastFiniteDifferenceError{1e-6};
    const auto tChecker =
        plato::test_utilities::GradientChecker{[](const double x) { return detail::smoothing_function(x); },
                                               [](const double x) { return detail::d_smoothing_function(x); }};
    constexpr auto tX = 0.74;
    constexpr auto tDirection = 1.0;

    EXPECT_NEAR(tChecker.maxFirstOrderTruncationError(tX, tDirection, tGradCheckParams), 0.0,
                tFirstOrderTruncationTolerance);

    const auto tErrors = tChecker.finiteDifferenceErrors(tX, tDirection, tGradCheckParams);
    ASSERT_FALSE(tErrors.empty());
    EXPECT_NEAR(tErrors.back().mValue, 0.0, tLastFiniteDifferenceError)
        << "Full gradient check:\n"
        << tChecker.table(tX, tDirection, tGradCheckParams);
}

TEST(OverhangCriterion, dOverhang)
{
    constexpr test_utilities::GradientCheckParameters tGradCheckParams{.5, 6, .0001};
    constexpr auto tFirstOrderTruncationTolerance{6e-1};
    constexpr auto tLastFiniteDifferenceError{1e-8};
    const auto tChecker = plato::test_utilities::GradientChecker{
        [](const double aNormalDotBuildDirection)
        { return detail::overhang(aNormalDotBuildDirection, kOverhangAngleThreshold, kStepTransitionWidth); },
        [](const double aNormalDotBuildDirection)
        { return detail::d_overhang(aNormalDotBuildDirection, kOverhangAngleThreshold, kStepTransitionWidth); }};
    const auto tX = kOverhangAngleThreshold - (kStepTransitionWidth / 2.0);
    constexpr auto tDirection = 1.0;

    EXPECT_NEAR(tChecker.maxFirstOrderTruncationError(tX, tDirection, tGradCheckParams), 0.0,
                tFirstOrderTruncationTolerance)
        << tChecker.table(tX, tDirection, tGradCheckParams);

    const auto tErrors = tChecker.finiteDifferenceErrors(tX, tDirection, tGradCheckParams);
    ASSERT_FALSE(tErrors.empty());
    EXPECT_NEAR(tErrors.back().mValue, 0.0, tLastFiniteDifferenceError)
        << "Full gradient check:\n"
        << tChecker.table(tX, tDirection, tGradCheckParams);
}

TEST(OverhangCriterion, SingleTriangleDerivative)
{
    constexpr auto tAbsoluteError = 7e-3;
    const auto tChecker = plato::test_utilities::GradientChecker{
        [](const linear_algebra::DynamicVector<double>& aTriNodalCoords)
        {
            const Triangle tTriangle{{aTriNodalCoords[0], aTriNodalCoords[1], aTriNodalCoords[2]},
                                     {aTriNodalCoords[3], aTriNodalCoords[4], aTriNodalCoords[5]},
                                     {aTriNodalCoords[6], aTriNodalCoords[7], aTriNodalCoords[8]},
                                     {1, 2, 3}};
            return detail::area_weighted_overhang_from_triangle(tTriangle, -std::sqrt(2.0) / 2.0, 0.05, {0, 0, 1});
        },
        [](const linear_algebra::DynamicVector<double>& aTriNodalCoords,
           const linear_algebra::DynamicVector<double>& aDirection)
        {
            const Triangle tTriangle{{aTriNodalCoords[0], aTriNodalCoords[1], aTriNodalCoords[2]},
                                     {aTriNodalCoords[3], aTriNodalCoords[4], aTriNodalCoords[5]},
                                     {aTriNodalCoords[6], aTriNodalCoords[7], aTriNodalCoords[8]},
                                     {1, 2, 3}};
            const auto tGradient = linear_algebra::DynamicVector<double>{
                detail::get_gradient_contribution_for_triangle(tTriangle, -std::sqrt(2.0) / 2.0, 0.05, {0, 0, 1})};
            return tGradient.dot(aDirection);
        }};
    const auto tGradientCheckParameters = plato::test_utilities::GradientCheckParameters{0.5, 10, .001};
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
    // 4 triangles as shown above with nodes {1,2,4}, {2,3,5}, {2,5,4}, {4,5,6}

    // clang-format on

    const std::vector<Triangle> tTriangles{{{0., 0., 0.}, {1., 0., 0.}, {.5, 1., 0.}, {1, 2, 4}},
                                           {{1., 0., 0.}, {2., 0., 0.}, {1.5, 1., 0.}, {2, 3, 5}},
                                           {{1., 0., 0.}, {1.5, 1., 0.}, {.5, 1., 0.}, {2, 5, 4}},
                                           {{.5, 1., 0.}, {1.5, 1., 0.}, {1., 2., 0.}, {4, 5, 6}}};

    const double tOverhangThreshold = -std::sqrt(2.0) / 2.0;
    constexpr double tTransitionWidth = 0.05;
    constexpr Vector3 tBuildDirection = {0, .681189886111555, -.732106781186548};
    constexpr size_t tNumNodes = 6;
    constexpr size_t tNumTris = 4;
    constexpr size_t tNumDimensions = 3;
    constexpr auto tAbsoluteError = 1e-10;

    // Calculate indivdual triangle maps--one for each tri
    std::vector<std::map<size_t, std::array<double, 3>>> tIndividualGradientMaps;
    for (size_t i = 0; i < tNumTris; ++i)
    {
        tIndividualGradientMaps.push_back(detail::calculate_gradient_map_from_triangles(
            {tTriangles[i]}, tOverhangThreshold, tTransitionWidth, tBuildDirection));
    }
    // Calculate combined triangle map from multiple tris
    std::map<size_t, std::array<double, 3>> tCombinedGradientMap = detail::calculate_gradient_map_from_triangles(
        tTriangles, tOverhangThreshold, tTransitionWidth, tBuildDirection);
    // Compare results
    for (size_t i = 0; i < tNumNodes; ++i)
    {
        std::array<double, 3> tCurNodeGradient = {0.0, 0.0, 0.0};
        // For this node get contributions from individual maps
        for (size_t j = 0; j < tNumTris; ++j)
        {
            if (tIndividualGradientMaps[j].count(i + 1))
            {
                for (size_t k = 0; k < tNumDimensions; ++k)
                {
                    tCurNodeGradient[k] += tIndividualGradientMaps[j].at(i + 1)[k];
                }
            }
        }
        // Compare against combined map
        for (size_t k = 0; k < tNumDimensions; ++k)
        {
            EXPECT_NEAR(tCurNodeGradient[k], tCombinedGradientMap.at(i + 1)[k], tAbsoluteError);
        }
    }
}

TEST(OverhangCriterion, FullGradientVectorFromPartialGradientMap)
{
    const std::map<size_t, std::array<double, 3>> tPartialGradientMap = {
        {3, {.1, .2, .3}}, {5, {.9, -.1, -.2}}, {9, {-1., -2., -3.}}};
    const std::vector<size_t> tAllNodeIDs = {2, 3, 5, 6, 7, 9, 11, 12};
    const std::vector<double> tFullGradientVector =
        detail::get_full_gradient_vector_from_gradient_map(tPartialGradientMap, tAllNodeIDs);
    const std::vector<double> tGoldValues = {0, 0, 0, .1,  .2,  .3,  .9, -.1, -.2, 0, 0, 0,
                                             0, 0, 0, -1., -2., -3., 0,  0,   0,   0, 0, 0};
    EXPECT_EQ(tFullGradientVector, tGoldValues);
}

TEST(OverhangCriterion, ParseInputDeck_Correct)
{
    const auto tMeshPath = std::filesystem::path{"temp_input_deck.txt"};
    std::ofstream tTextFile(tMeshPath);
    tTextFile << "<OverhangInput>\n";
    tTextFile << "  <BuildDirection>0 0 1</BuildDirection>\n";
    tTextFile << "  <OverhangAngleFromHorizontal>45</OverhangAngleFromHorizontal>\n";
    tTextFile << "  <TransitionWidth>0.1</TransitionWidth>\n";
    tTextFile << "</OverhangInput>";
    tTextFile.close();

    ParsedInputParams tParams = detail::parse_input_deck(tMeshPath);
    constexpr auto tAbsoluteError = 1e-10;
    EXPECT_NEAR(tParams.build_direction.x, 0.0, tAbsoluteError);
    EXPECT_NEAR(tParams.build_direction.y, 0.0, tAbsoluteError);
    EXPECT_NEAR(tParams.build_direction.z, 1.0, tAbsoluteError);
    EXPECT_NEAR(tParams.overhang_angle_threshold, -std::sqrt(2.0) / 2.0, tAbsoluteError);
    EXPECT_NEAR(tParams.transition_width, 0.1, tAbsoluteError);

    std::filesystem::remove(tMeshPath);
}

TEST(OverhangCriterion, ParseInputDeck_WrongNumberBuildDirectionParameters)
{
    const auto tMeshPath = std::filesystem::path{"temp_input_deck.txt"};
    std::ofstream tTextFile(tMeshPath);
    tTextFile << "<OverhangInput>\n";
    tTextFile << "  <BuildDirection>0</BuildDirection>\n";
    tTextFile << "  <OverhangAngleFromHorizontal>45</OverhangAngleFromHorizontal>\n";
    tTextFile << "  <TransitionWidth>0.1</TransitionWidth>\n";
    tTextFile << "</OverhangInput>";
    tTextFile.close();

    ParsedInputParams tParams;
    EXPECT_THROW(tParams = detail::parse_input_deck(tMeshPath), plato::utilities::Exception);
    std::filesystem::remove(tMeshPath);
}

TEST(OverhangCriterion, ParseInputDeck_MissingBuildDirectionParameters)
{
    const auto tMeshPath = std::filesystem::path{"temp_input_deck.txt"};
    std::ofstream tTextFile(tMeshPath);
    tTextFile << "<OverhangInput>\n";
    tTextFile << "  <OverhangAngleFromHorizontal>45</OverhangAngleFromHorizontal>\n";
    tTextFile << "  <TransitionWidth>0.1</TransitionWidth>\n";
    tTextFile << "</OverhangInput>";
    tTextFile.close();

    ParsedInputParams tParams;
    EXPECT_THROW(tParams = detail::parse_input_deck(tMeshPath), std::runtime_error);
    std::filesystem::remove(tMeshPath);
}

TEST(OverhangCriterion, ParseInputDeck_MissingOverhangAngleParameter)
{
    const auto tMeshPath = std::filesystem::path{"temp_input_deck.txt"};
    std::ofstream tTextFile(tMeshPath);
    tTextFile << "<OverhangInput>\n";
    tTextFile << "  <BuildDirection>0 0 1</BuildDirection>\n";
    tTextFile << "  <TransitionWidth>0.1</TransitionWidth>\n";
    tTextFile << "</OverhangInput>";
    tTextFile.close();

    ParsedInputParams tParams;
    EXPECT_THROW(tParams = detail::parse_input_deck(tMeshPath), std::runtime_error);
    std::filesystem::remove(tMeshPath);
}

TEST(OverhangCriterion, ParseInputDeck_MissingTransitionWidthParameter)
{
    const auto tMeshPath = std::filesystem::path{"temp_input_deck.txt"};
    std::ofstream tTextFile(tMeshPath);
    tTextFile << "<OverhangInput>\n";
    tTextFile << "  <OverhangAngleFromHorizontal>45</OverhangAngleFromHorizontal>\n";
    tTextFile << "  <BuildDirection>0 0 1</BuildDirection>\n";
    tTextFile << "</OverhangInput>";
    tTextFile.close();

    ParsedInputParams tParams;
    EXPECT_THROW(tParams = detail::parse_input_deck(tMeshPath), std::runtime_error);
    std::filesystem::remove(tMeshPath);
}

}  // namespace plato::criteria::extension::unittest
