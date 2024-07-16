#include <gtest/gtest.h>

#include <cmath>
#include <filesystem>

#include "plato/third_party_integration/common/unittest/CoordinateTestUtilities.hpp"
#include "plato/third_party_integration/stk_io/Triangle.hpp"

namespace plato::third_party_integration::stk_io::unittest
{
namespace
{
Triangle create_unit_triangle()
{
    const common::Coordinate tPointA{1, 0, 0};
    const common::Coordinate tPointB{0, 1, 0};
    const common::Coordinate tPointC{0, 0, 1};

    return Triangle{tPointA, tPointB, tPointC};
}
}  // namespace

TEST(Triangle, Volume)
{
    const Triangle tTri = create_unit_triangle();
    const double tResult = tTri.volume();

    const common::Vector3 tVectorA{tTri.p1 - tTri.p0};
    const common::Vector3 tVectorB{tTri.p2 - tTri.p0};
    const common::Vector3 tCrossBA = cross(tVectorB, tVectorA);
    const double tGold = std::sqrt(dot(tCrossBA, tCrossBA)) * 0.5;

    EXPECT_DOUBLE_EQ(tResult, tGold);
}

TEST(Triangle, Centroid)
{
    const Triangle tTri = create_unit_triangle();
    const common::Coordinate tResult = tTri.centroid();

    const common::Coordinate tGold{1.0 / 3.0, 1.0 / 3.0, 1.0 / 3.0};
    common::unittest::test_double_equality_of_components(tResult, tGold, TEST_CONTEXT("Triangle centroid"));
}

}  // namespace plato::third_party_integration::stk_io::unittest
