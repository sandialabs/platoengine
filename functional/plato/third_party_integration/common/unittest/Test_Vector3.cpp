#include <gtest/gtest.h>

#include <filesystem>

#include "plato/third_party_integration/common/Vector3.hpp"
#include "plato/third_party_integration/common/unittest/CoordinateTestUtilities.hpp"

namespace plato::third_party_integration::common::unittest
{
namespace
{
constexpr Vector3 kX{1, 0, 0};
constexpr Vector3 kY{0, 1, 0};
constexpr Vector3 kZ{0, 0, 1};
constexpr Vector3 k123{1, 2, 3};

template <typename Container>
void test_flatten()
{
    constexpr Container tContainer{1, 2, 3};
    {
        constexpr int tNumberOfSpatialDimensions = 2;
        const auto tResult = flatten(tContainer, tNumberOfSpatialDimensions);
        const std::vector<double> tGold = {1, 2};
        EXPECT_EQ(tResult, tGold);
    }
    {
        constexpr int tNumberOfSpatialDimensions = 3;
        const auto tResult = flatten(tContainer, tNumberOfSpatialDimensions);
        const std::vector<double> tGold = {1, 2, 3};
        EXPECT_EQ(tResult, tGold);
    }
}

}  // namespace

TEST(Vector3, CoordinateSubtraction)
{
    constexpr Coordinate p1{1, 2, 3};
    constexpr Coordinate p2{4, 5, 6};
    constexpr Vector3 tResult = p2 - p1;
    constexpr Vector3 tGold{3, 3, 3};

    test_double_equality_of_components(tResult, tGold, TEST_CONTEXT("Vector3 subtraction"));
}

TEST(Vector3, CoordinateAddition)
{
    constexpr Coordinate p1{38, 21, 86};
    constexpr Coordinate p2{71, 91, 10};
    {
        constexpr Coordinate tResult = p1 + p2;
        constexpr Coordinate tGold{109, 112, 96};
        test_double_equality_of_components(tResult, tGold, TEST_CONTEXT("Coordinate addition"));
    }

    {
        constexpr Coordinate tResult = p1 + p2 + p1;
        constexpr Coordinate tGold{147, 133, 182};
        test_double_equality_of_components(tResult, tGold, TEST_CONTEXT("Coordinate addition"));
    }
}

TEST(Vector3, CoordinateScalarMultiplication)
{
    constexpr Coordinate p1{1, 2, 3};
    constexpr double tScale{10};
    constexpr Coordinate tResult = p1 * tScale;
    constexpr Coordinate tGold{10, 20, 30};

    test_double_equality_of_components(tResult, tGold, TEST_CONTEXT("Coordinate multiplication"));
}

TEST(Vector3, CoordinateScalarDivision)
{
    constexpr Coordinate p1{10, 20, 30};
    constexpr double tDivisor{10};
    constexpr Coordinate tResult = p1 / tDivisor;
    constexpr Coordinate tGold{1, 2, 3};

    test_double_equality_of_components(tResult, tGold, TEST_CONTEXT("Vector3 scalar division"));
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
        test_double_equality_of_components(tResult, kZ, TEST_CONTEXT("Vector3 cross product"));
    }
    {
        constexpr Vector3 tResult = cross(kY, k123);
        constexpr Vector3 tGold{3, 0, -1};
        test_double_equality_of_components(tResult, tGold, TEST_CONTEXT("Vector3 cross product"));
    }
    {
        constexpr Vector3 tResult = cross(k123, k123);
        constexpr Vector3 tGold{0, 0, 0};
        test_double_equality_of_components(tResult, tGold, TEST_CONTEXT("Vector3 cross product"));
    }
}

TEST(Coordinate, Magnitude)
{
    constexpr Coordinate tCoordinate{-1, -2, -3};
    const double tGold = std::sqrt(1 + 4 + 9);
    EXPECT_EQ(tGold, magnitude(tCoordinate));
}

TEST(Vector3, Magnitude)
{
    constexpr Vector3 tVector{1, 2, 3};
    const double tGold = std::sqrt(1 + 4 + 9);
    EXPECT_EQ(tGold, magnitude(tVector));
}

TEST(Container, ContainerToVector)
{
    test_flatten<Coordinate>();
    test_flatten<Vector3>();
}

}  // namespace plato::third_party_integration::common::unittest
