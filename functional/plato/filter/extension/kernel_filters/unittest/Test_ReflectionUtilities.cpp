#include <gtest/gtest.h>

#include <boost/math/constants/constants.hpp>
#include <cmath>

#include "plato/filter/extension/kernel_filters/Plane.hpp"
#include "plato/filter/extension/kernel_filters/ReflectionUtilities.hpp"
#include "plato/third_party_integration/common/test_utilities/CoordinateTestUtilities.hpp"
#include "plato/utilities/Zip.hpp"

namespace plato::filter::extension::kernel_filters::unittest
{

namespace
{
const auto kPoint = third_party_integration::common::Coordinate{1, 2, 3};
const auto kPointTwo = third_party_integration::common::Coordinate{-4, -5, -6};
const auto kPositiveXPlane = Plane{{0, 0, 0}, {1, 0, 0}};
const auto kPositiveYPlane = Plane{{0, 0, 0}, {0, 1, 0}};

const auto kNegativeZPlane = Plane{{0, 0, 0}, {0, 0, -1}};
const auto kSqrtTwoOverTwo = std::sqrt(2.0) / 2.0;
const auto kPointWithID = third_party_integration::stk_search::SearchPointWithIdentifier{
    third_party_integration::stk_search::SearchPoint{1, 2, 3}, third_party_integration::stk_search::Identifier{2, 3}};

}  // namespace

namespace
{
void check_reflected_points_against_gold(const std::vector<third_party_integration::common::Coordinate>& aResultPoints,
                                         const std::vector<third_party_integration::common::Coordinate>& aGoldPoints,
                                         const test_utilities::TestContext& aTestContext)
{
    ASSERT_EQ(aResultPoints.size(), aGoldPoints.size());
    for (const auto& [tResult, tGold] : utilities::Zip(aResultPoints, aGoldPoints))
    {
        third_party_integration::common::test_utilities::test_double_equality_of_components(tResult, tGold,
                                                                                            aTestContext);
    }
}

}  // namespace

TEST(ReflectionUtilities, ReflectionList)
{
    {
        const auto tPoints = std::vector<third_party_integration::common::Coordinate>{kPoint};
        const auto tResultPoints = detail::reflect_points_in_planes(tPoints, kNegativeZPlane);
        const auto tGoldPoints = detail::reflect_point_in_single_plane(tPoints, kNegativeZPlane);
        check_reflected_points_against_gold(tResultPoints, tGoldPoints, TEST_CONTEXT("-Z plane reflection"));
    }

    {
        const auto tVectorOfOnePoint = std::vector<third_party_integration::common::Coordinate>{kPoint};
        const auto tResultPoints =
            detail::reflect_points_in_planes(tVectorOfOnePoint, kPositiveXPlane, kPositiveYPlane);
        const auto tGoldPoints = detail::reflect_point_in_single_plane(
            detail::reflect_point_in_single_plane(tVectorOfOnePoint, kPositiveXPlane), kPositiveYPlane);
        check_reflected_points_against_gold(tResultPoints, tGoldPoints, TEST_CONTEXT("X plane, Y plane reflection"));
    }

    {
        const auto tVectorOfOnePoint = std::vector<third_party_integration::common::Coordinate>{kPoint};
        const auto tResultPoints =
            detail::reflect_points_in_planes(tVectorOfOnePoint, kPositiveXPlane, kPositiveYPlane, kNegativeZPlane);
        const auto tGoldPoints = detail::reflect_point_in_single_plane(
            detail::reflect_point_in_single_plane(
                detail::reflect_point_in_single_plane(tVectorOfOnePoint, kPositiveXPlane), kPositiveYPlane),
            kNegativeZPlane);
        check_reflected_points_against_gold(tResultPoints, tGoldPoints, TEST_CONTEXT("X,Y,-Z plane reflections"));
    }
    {
        namespace tpis = third_party_integration::stk_search;
        const auto tGoldPoint = kPositiveXPlane.reflect(kPoint);
        const auto tReflectedPoints = detail::reflect_points_in_planes(
            std::vector<tpis::SearchPointWithIdentifier>{kPointWithID}, kPositiveXPlane);
        ASSERT_EQ(tReflectedPoints.size(), 2U);

        EXPECT_EQ(tReflectedPoints.front().second, kPointWithID.second)
            << "Identifier remains the same under reflection";
        EXPECT_EQ(tReflectedPoints.back().second, kPointWithID.second)
            << "Identifier remains the same under reflection";

        check_reflected_points_against_gold({tpis::convert_search_point(tReflectedPoints.back().first)}, {tGoldPoint},
                                            TEST_CONTEXT("STK Point with ID xplane reflection"));
    }
}

TEST(ReflectionUtilities, SingleReflectionImpl)
{
    const auto tResultPoints = detail::reflect_point_in_single_plane_impl(
        std::vector<third_party_integration::common::Coordinate>{kPoint, kPointTwo}, kNegativeZPlane);
    const std::vector<third_party_integration::common::Coordinate> tGoldCoordinates = {{1, 2, -3}, {-4, -5, 6}};

    check_reflected_points_against_gold(tResultPoints, tGoldCoordinates, TEST_CONTEXT("-Z plane reflection"));
}

TEST(ReflectionUtilities, SingleReflection)
{
    const auto tResultPoints = detail::reflect_point_in_single_plane(
        std::vector<third_party_integration::common::Coordinate>{kPoint, kPointTwo}, kNegativeZPlane);
    const std::vector<third_party_integration::common::Coordinate> tGoldCoordinates = {
        kPoint, kPointTwo, {1, 2, -3}, {-4, -5, 6}};

    check_reflected_points_against_gold(tResultPoints, tGoldCoordinates, TEST_CONTEXT("-Z plane reflection"));
}

TEST(ReflectionUtilities, ReflectedPointsInMirroredWedge)
{
    const auto tRepeats = 3U;
    const auto tWedge = Wedge{kPositiveYPlane, kPositiveXPlane, tRepeats};
    const auto tResultPoints = detail::reflect_points_in_mirrored_wedge<third_party_integration::common::Coordinate>(
        {kPoint, kPointTwo}, tWedge);

    const std::vector<third_party_integration::common::Coordinate> tGoldCoordinates = {
        {1, 2, 3}, {-4, -5, -6}, {1, -2, 3}, {-4, 5, -6}, {-1, -2, 3}, {4, 5, -6}, {-1, 2, 3}, {4, -5, -6}};

    check_reflected_points_against_gold(tResultPoints, tGoldCoordinates, TEST_CONTEXT("Y, X plane reflections"));
}

TEST(ReflectionUtilities, MinimumDistanceReflectedPointsWedge)
{
    const auto tRepeats = 3U;
    const auto tWedge = Wedge{kPositiveYPlane, kPositiveXPlane, tRepeats};
    const auto tTestPoint = third_party_integration::common::Coordinate{-1, -2, 4};
    const auto tShortestDistance =
        minimum_distance_reflected_points_wedge(SourcePoint{kPoint}, TargetPoint{tTestPoint}, tWedge);
    EXPECT_DOUBLE_EQ(tShortestDistance, 1.0);
}

TEST(ReflectionUtilities, MinimumDistanceReflectedPointsPlaneList)
{
    {
        const auto tTestPoint = third_party_integration::common::Coordinate{1, -2, 3};
        const auto tResult = minimum_distance_reflected_points_planes_list(SourcePoint{kPoint}, TargetPoint{tTestPoint},
                                                                           kPositiveYPlane);
        const auto tGold = 0.0;
        EXPECT_EQ(tResult, tGold);
    }
    {
        const auto tTestPoint = third_party_integration::common::Coordinate{-1, 2, -2};
        const auto tResult = minimum_distance_reflected_points_planes_list(
            SourcePoint{kPoint}, TargetPoint{tTestPoint}, kPositiveYPlane, kPositiveXPlane, kNegativeZPlane);
        const auto tGold = 1.0;
        EXPECT_EQ(tResult, tGold);
    }
}

}  // namespace plato::filter::extension::kernel_filters::unittest
