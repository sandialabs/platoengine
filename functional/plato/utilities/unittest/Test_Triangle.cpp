#include <gtest/gtest.h>

#include <cmath>
#include <filesystem>

#include "plato/utilities/Triangle.hpp"

namespace plato::utilities::unittest
{
TEST(Triangle, Volume)
{
    Coordinate tPointA{1, 0, 0};
    Coordinate tPointB{0, 1, 0};
    Coordinate tPointC{0, 0, 1};

    const Triangle tTri{tPointA, tPointB, tPointC};
    const double tResult = tTri.volume();

    const Vector3 tVectorA{tPointB - tPointA};
    const Vector3 tVectorB{tPointC - tPointA};
    const Vector3 tCrossBA = cross(tVectorB, tVectorA);
    const double tGold = std::sqrt(dot(tCrossBA, tCrossBA)) * 0.5;

    EXPECT_DOUBLE_EQ(tResult, tGold);
}

}  // namespace plato::utilities::unittest
