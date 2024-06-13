#include <gtest/gtest.h>

#include <filesystem>

#include "plato/utilities/Vector3.hpp"

namespace plato::utilities::unittest
{
namespace
{
constexpr Vector3 kX{1, 0, 0};
constexpr Vector3 kY{0, 1, 0};
constexpr Vector3 kZ{0, 0, 1};
constexpr Vector3 k123{1, 2, 3};

void test_double_equality_of_components(const Vector3& aResult, const Vector3& aGold)
{
    EXPECT_DOUBLE_EQ(aResult.x, aGold.x);
    EXPECT_DOUBLE_EQ(aResult.y, aGold.y);
    EXPECT_DOUBLE_EQ(aResult.z, aGold.z);
}

}  // namespace

TEST(Vector3, CoordinateSubtraction)
{
    constexpr Coordinate p1{1, 2, 3};
    constexpr Coordinate p2{4, 5, 6};
    constexpr Vector3 tResult = p2 - p1;
    constexpr Vector3 tGold{3, 3, 3};

    test_double_equality_of_components(tResult, tGold);
}

TEST(Vector3, Dot)
{
    {
        constexpr double tResult = dot(kX, kY);
        constexpr double tGold = 0.0;
        EXPECT_DOUBLE_EQ(tResult, tGold);
    }
    {
        constexpr double tResult = dot(kZ, k123);
        constexpr double tGold = 3.0;
        EXPECT_DOUBLE_EQ(tResult, tGold);
    }
    {
        constexpr double tResult = dot(k123, k123);
        constexpr double tGold = 14.0;
        EXPECT_DOUBLE_EQ(tResult, tGold);
    }
}

TEST(Vector3, Cross)
{
    {
        constexpr Vector3 tResult = cross(kX, kY);
        test_double_equality_of_components(tResult, kZ);
    }
    {
        constexpr Vector3 tResult = cross(kY, k123);
        constexpr Vector3 tGold{3, 0, -1};
        test_double_equality_of_components(tResult, tGold);
    }
    {
        constexpr Vector3 tResult = cross(k123, k123);
        constexpr Vector3 tGold{0, 0, 0};
        test_double_equality_of_components(tResult, tGold);
    }
}
}  // namespace plato::utilities::unittest
