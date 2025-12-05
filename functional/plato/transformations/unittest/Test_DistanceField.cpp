#include <gtest/gtest.h>

#include "plato/transformations/DistanceField.hpp"

namespace plato::transformations::unittest
{
namespace
{
namespace tpic = third_party_integration::common;
constexpr auto kTestPoint = tpic::Coordinate{.x = 10.0, .y = 11.0, .z = 12.0};
}  // namespace

TEST(DistanceField, PointPlaneDistanceZeroOffsetCartesianNormals)
{
    const auto tXPlane = Plane{.mOriginSignedDistance = 0.0, .mNormal = {.x = 1.0, .y = 0.0, .z = 0.0}};
    const auto tYPlane = Plane{.mOriginSignedDistance = 0.0, .mNormal = {.x = 0.0, .y = 1.0, .z = 0.0}};
    const auto tZPlane = Plane{.mOriginSignedDistance = 0.0, .mNormal = {.x = 0.0, .y = 0.0, .z = 1.0}};
    {
        EXPECT_EQ(point_plane_signed_distance(tXPlane, kTestPoint), kTestPoint.x);
        EXPECT_EQ(point_plane_signed_distance(tYPlane, kTestPoint), kTestPoint.y);
        EXPECT_EQ(point_plane_signed_distance(tZPlane, kTestPoint), kTestPoint.z);
    }
    {
        constexpr auto tTestPointBelow = tpic::Coordinate{.x = -10.0, .y = -11.0, .z = -12.0};
        EXPECT_EQ(point_plane_signed_distance(tXPlane, tTestPointBelow), tTestPointBelow.x);
        EXPECT_EQ(point_plane_signed_distance(tYPlane, tTestPointBelow), tTestPointBelow.y);
        EXPECT_EQ(point_plane_signed_distance(tZPlane, tTestPointBelow), tTestPointBelow.z);
    }
}

TEST(DistanceField, PointPlaneDistanceNonZeroOffsetCartesianNormals)
{
    constexpr auto tOriginDistance = -5.0;
    constexpr auto tXPlane = Plane{.mOriginSignedDistance = tOriginDistance, .mNormal = {.x = 1.0, .y = 0.0, .z = 0.0}};
    constexpr auto tYPlane = Plane{.mOriginSignedDistance = tOriginDistance, .mNormal = {.x = 0.0, .y = 1.0, .z = 0.0}};
    constexpr auto tZPlane = Plane{.mOriginSignedDistance = tOriginDistance, .mNormal = {.x = 0.0, .y = 0.0, .z = 1.0}};

    {
        EXPECT_EQ(point_plane_signed_distance(tXPlane, kTestPoint), kTestPoint.x + tOriginDistance);
        EXPECT_EQ(point_plane_signed_distance(tYPlane, kTestPoint), kTestPoint.y + tOriginDistance);
        EXPECT_EQ(point_plane_signed_distance(tZPlane, kTestPoint), kTestPoint.z + tOriginDistance);

        // Point on plane
        constexpr auto tPointOnXPlane = tpic::Coordinate{.x = 5.0, .y = 10.0, .z = -3.0};
        EXPECT_EQ(point_plane_signed_distance(tXPlane, tPointOnXPlane), 0.0);
    }
    {
        constexpr auto tTestPointBelow = tpic::Coordinate{.x = -10.0, .y = -11.0, .z = -12.0};
        EXPECT_EQ(point_plane_signed_distance(tXPlane, tTestPointBelow), tTestPointBelow.x + tOriginDistance);
        EXPECT_EQ(point_plane_signed_distance(tYPlane, tTestPointBelow), tTestPointBelow.y + tOriginDistance);
        EXPECT_EQ(point_plane_signed_distance(tZPlane, tTestPointBelow), tTestPointBelow.z + tOriginDistance);

        // Point on plane
        constexpr auto tPointOnYPlane = tpic::Coordinate{.x = 15.0, .y = 5.0, .z = -3.0};
        EXPECT_EQ(point_plane_signed_distance(tYPlane, tPointOnYPlane), 0.0);
    }
}

TEST(DistanceField, PointPlaneDistanceArbitrary)
{
    constexpr auto tOriginDistance = 30.0;
    constexpr auto tNormal = tpic::Vector3{.x = 3.0, .y = 0.0, .z = -4.0};
    constexpr auto tNormalMagnitude = 5.0;
    constexpr auto tPlane = Plane{.mOriginSignedDistance = tOriginDistance, .mNormal = tNormal};
    constexpr auto tOrigin = tpic::Coordinate{.x = 0.0, .y = 0.0, .z = 0.0};
    constexpr auto tExpected = tpic::dot(tNormal, kTestPoint - tOrigin) / tNormalMagnitude + tOriginDistance;
    EXPECT_DOUBLE_EQ(point_plane_signed_distance(tPlane, kTestPoint), tExpected);

    // Distance to origin
    EXPECT_DOUBLE_EQ(point_plane_signed_distance(tPlane, tOrigin), tOriginDistance);

    // Point on plane
    constexpr auto tPointOnPlane = tpic::Coordinate{0.0, 0.0, -tOriginDistance * tNormalMagnitude / tNormal.z};
    EXPECT_DOUBLE_EQ(point_plane_signed_distance(tPlane, tPointOnPlane), 0.0);
}
}  // namespace plato::transformations::unittest
