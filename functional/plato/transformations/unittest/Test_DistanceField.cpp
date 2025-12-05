#include <gtest/gtest.h>

#include "plato/transformations/DistanceField.hpp"

namespace plato::transformations::unittest
{
namespace
{
namespace tpic = third_party_integration::common;
constexpr auto kTestPoint = tpic::Coordinate{.x=10.0, .y=11.0, .z=12.0};
}

TEST(DistanceField, PointPlaneDistanceZeroOffsetCartesianNormals)
{
    const auto tXPlane = Plane{.mOriginDistance = 0.0, .mNormal = {.x = 1.0, .y = 0.0, .z = 0.0}};
    EXPECT_EQ(point_plane_distance(tXPlane, kTestPoint), kTestPoint.x);

    const auto tYPlane = Plane{.mOriginDistance = 0.0, .mNormal = {.x = 0.0, .y = 1.0, .z = 0.0}};
    EXPECT_EQ(point_plane_distance(tYPlane, kTestPoint), kTestPoint.y);

    const auto tZPlane = Plane{.mOriginDistance = 0.0, .mNormal = {.x = 0.0, .y = 0.0, .z = 1.0}};
    EXPECT_EQ(point_plane_distance(tZPlane, kTestPoint), kTestPoint.z);
}

TEST(DistanceField, PointPlaneDistanceNonZeroOffsetCartesianNormals)
{
    constexpr auto tOriginDistance = 5.0;
    constexpr auto tXPlane = Plane{.mOriginDistance = tOriginDistance, .mNormal = {.x = 1.0, .y = 0.0, .z = 0.0}};
    EXPECT_EQ(point_plane_distance(tXPlane, kTestPoint), kTestPoint.x + tOriginDistance);

    constexpr  auto tYPlane = Plane{.mOriginDistance = tOriginDistance, .mNormal = {.x = 0.0, .y = 1.0, .z = 0.0}};
    EXPECT_EQ(point_plane_distance(tYPlane, kTestPoint), kTestPoint.y + tOriginDistance);

    constexpr  auto tZPlane = Plane{.mOriginDistance = tOriginDistance, .mNormal = {.x = 0.0, .y = 0.0, .z = 1.0}};
    EXPECT_EQ(point_plane_distance(tZPlane, kTestPoint), kTestPoint.z + tOriginDistance);
}

TEST(DistanceField, PointPlaneDistanceArbitrary)
{
    constexpr auto tOriginDistance = -30.0;
    constexpr auto tNormal = tpic::Vector3{.x = 1.0, .y = -1.0, .z = 2.0};
    constexpr auto tPlane = Plane{.mOriginDistance = tOriginDistance, .mNormal = tNormal};
    constexpr auto tOrigin = tpic::Coordinate{.x = 0.0, .y = 0.0, .z = 0.0};
    const auto tExpected = tpic::dot(tNormal, kTestPoint - tOrigin) / magnitude(tNormal) + tOriginDistance;
    EXPECT_EQ(point_plane_distance(tPlane, kTestPoint), tExpected);

    // Distance to origin
    EXPECT_EQ(point_plane_distance(tPlane, tOrigin), tOriginDistance);
}
}
