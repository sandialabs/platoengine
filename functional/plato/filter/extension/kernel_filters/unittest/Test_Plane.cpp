#include <gtest/gtest.h>

#include <numbers>

#include "plato/filter/extension/kernel_filters/Plane.hpp"
#include "plato/third_party_integration/common/test_utilities/CoordinateTestUtilities.hpp"

namespace plato::filter::extension::kernel_filters::unittest
{

namespace
{
constexpr auto kPoint = third_party_integration::common::Coordinate{1, 2, 3};
const auto kPositiveXPlane = Plane({0, 0, 0}, {1, 0, 0});
const auto kNegativeZPlane = Plane({0, 0, 0}, {0, 0, -1});
constexpr auto kSqrtTwoOverTwo = std::numbers::sqrt2 / 2.0;
const auto kFortyFive = Plane({0.5, 0.5, 0}, {kSqrtTwoOverTwo, kSqrtTwoOverTwo, 0});
constexpr auto kTolerance = 1e-15;

const auto kPointWithID = third_party_integration::stk_search::SearchPointWithIdentifier{
    third_party_integration::stk_search::SearchPoint{1, 2, 3}, third_party_integration::stk_search::Identifier{2, 3}};

}  // namespace

TEST(Plane, ReflectCoordinate)
{
    {
        constexpr auto tGoldPoint = third_party_integration::common::Coordinate{1, 2, -3};
        const auto tResult = kNegativeZPlane.reflect(kPoint);
        third_party_integration::common::test_utilities::test_double_equality_of_components(
            tResult, tGoldPoint, TEST_CONTEXT("XPlane reflection point"));
    }
    {
        constexpr auto tTestPoint = third_party_integration::common::Coordinate{1, 1, 3};
        constexpr auto tGoldPoint = third_party_integration::common::Coordinate{0, 0, 3};
        const auto tResult = kFortyFive.reflect(tTestPoint);
        third_party_integration::common::test_utilities::test_near_equality_of_components(
            tResult, tGoldPoint, kTolerance, TEST_CONTEXT("Forty Five Plane reflection point"));
    }
    {
        const auto tGoldPoint = kPositiveXPlane.reflect(kPoint);
        const auto tResult = kPositiveXPlane.reflect(kPointWithID);
        const auto tID = tResult.second;
        EXPECT_EQ(kPointWithID.second, tID);
        const auto tCoordinate = third_party_integration::stk_search::convert_search_point(tResult.first);
        third_party_integration::common::test_utilities::test_near_equality_of_components(
            tCoordinate, tGoldPoint, kTolerance, TEST_CONTEXT("Forty Five Plane reflection point"));
    }
}

TEST(Plane, ReflectPlane)
{
    constexpr auto tTolerance = 1e-15;
    const auto tPositiveYPlane = Plane({0.5, 0.5, 0}, {0, 1, 0});
    const auto tFortyFivePlane = Plane({0, 0, 0}, {-kSqrtTwoOverTwo, kSqrtTwoOverTwo, 0});

    const auto tResult = tFortyFivePlane.reflect(tPositiveYPlane);
    const auto tGold = Plane({0.5, 0.5, 0}, {1, 0, 0});

    namespace tpic = third_party_integration::common;
    tpic::test_utilities::test_near_equality_of_components(static_cast<tpic::Vector3>(tResult.mUnitNormal),
                                                           static_cast<tpic::Vector3>(tGold.mUnitNormal), tTolerance,
                                                           TEST_CONTEXT("Plane normal."));
    tpic::test_utilities::test_near_equality_of_components(tResult.mCenter, tGold.mCenter, tTolerance,
                                                           TEST_CONTEXT("Plane center."));
}

}  // namespace plato::filter::extension::kernel_filters::unittest
