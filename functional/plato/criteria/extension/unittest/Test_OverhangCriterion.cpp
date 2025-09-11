#include <gtest/gtest.h>
/*
#include <string_view>

#include "plato/mesh/Mesh.hpp"
#include "plato/test_utilities/FilesystemTestUtility.hpp"
#include "plato/test_utilities/TestContext.hpp"
#include "plato/third_party_integration/stk_io/CommandGenerator.hpp"
#include "plato/third_party_integration/stk_io/WriteUtilities.hpp"
*/
#include "plato/criteria/extension/OverhangCriterion.hpp"
#include "plato/utilities/Zip.hpp"

namespace plato::criteria::extension::unittest
{
using namespace plato::third_party_integration::common;
using namespace plato::third_party_integration::stk_io;

constexpr double kTolerance{1e-14};
constexpr Vector3 kBuildDirection{0, 0, -1};
constexpr double kStepTransitionWidth{0.05};
constexpr double kOverhangAngleThreshold{-std::sqrt(2.0) / 2.0};
// namespace
//{

// constexpr std::string_view kBrickFile = "brick.exo";
// constexpr std::string_view kTestFile = "test.exo";
// }  // namespace

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

}  // namespace plato::criteria::extension::unittest
