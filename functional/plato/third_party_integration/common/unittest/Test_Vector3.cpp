#include <gtest/gtest.h>

#include <filesystem>

#include "plato/test_utilities/TestContext.hpp"
#include "plato/third_party_integration/common/Vector3.hpp"
#include "plato/third_party_integration/common/test_utilities/CoordinateTestUtilities.hpp"

namespace plato::third_party_integration::common::unittest
{
namespace
{
constexpr Vector3 kX{1, 0, 0};
constexpr Vector3 kY{0, 1, 0};
constexpr Vector3 kZ{0, 0, 1};
constexpr Vector3 k123{1, 2, 3};
}  // namespace

TEST(Vector3, CoordinateSubtraction)
{
    constexpr Coordinate p1{1, 2, 3};
    constexpr Coordinate p2{4, 5, 6};
    constexpr Vector3 tResult = p2 - p1;
    constexpr Vector3 tGold{3, 3, 3};

    test_utilities::test_double_equality_of_components(tResult, tGold, TEST_CONTEXT("Vector3 subtraction"));
}

TEST(Vector3, CoordinateAddition)
{
    constexpr Coordinate p1{38, 21, 86};
    constexpr Coordinate p2{71, 91, 10};
    {
        constexpr Coordinate tResult = p1 + p2;
        constexpr Coordinate tGold{109, 112, 96};
        test_utilities::test_double_equality_of_components(tResult, tGold, TEST_CONTEXT("Coordinate addition"));
    }

    {
        constexpr Coordinate tResult = p1 + p2 + p1;
        constexpr Coordinate tGold{147, 133, 182};
        test_utilities::test_double_equality_of_components(tResult, tGold, TEST_CONTEXT("Coordinate addition"));
    }
}

TEST(Vector3, CoordinateScalarMultiplication)
{
    constexpr Coordinate p1{1, 2, 3};
    constexpr double tScale{10};
    constexpr Coordinate tResult = p1 * tScale;
    constexpr Coordinate tGold{10, 20, 30};

    test_utilities::test_double_equality_of_components(tResult, tGold, TEST_CONTEXT("Coordinate multiplication"));
}

TEST(Vector3, CoordinateScalarDivision)
{
    constexpr Coordinate p1{10, 20, 30};
    constexpr double tDivisor{10};
    constexpr Coordinate tResult = p1 / tDivisor;
    constexpr Coordinate tGold{1, 2, 3};

    test_utilities::test_double_equality_of_components(tResult, tGold, TEST_CONTEXT("Vector3 scalar division"));
}

TEST(Vector3, CoordinateScalarAddition)
{
    constexpr Coordinate p1{10, 20, 30};
    constexpr double tScalar{10};
    constexpr Coordinate tResult = p1 + tScalar;
    constexpr Coordinate tGold{20, 30, 40};

    test_utilities::test_double_equality_of_components(tResult, tGold, TEST_CONTEXT("Vector3 scalar addition"));
}

TEST(Vector3, CoordinateCoordinateDivision)
{
    constexpr Coordinate p1{10, 20, 30};
    constexpr Coordinate tDivisor{2.0, 4.0, 10.0};
    constexpr Coordinate tResult = p1 / tDivisor;
    constexpr Coordinate tGold{5.0, 5.0, 3.0};

    test_utilities::test_double_equality_of_components(tResult, tGold, TEST_CONTEXT("Coordinate coordinate division"));
}

TEST(Vector3, ScalarCoordinateDivision)
{
    constexpr double scalar{10};
    constexpr Coordinate tDivisor{2.0, 5.0, 1.0};
    constexpr Coordinate tResult = scalar / tDivisor;
    constexpr Coordinate tGold{5.0, 2.0, 10.0};

    test_utilities::test_double_equality_of_components(tResult, tGold, TEST_CONTEXT("Scalar coordinate division"));
}

TEST(Vector3, CoordinateCoordinateMultiply)
{
    constexpr Coordinate p1{10, 20, 30};
    constexpr Coordinate tMultiplier{2.0, 4.0, 10.0};
    constexpr Coordinate tResult = p1 * tMultiplier;
    constexpr Coordinate tGold{20.0, 80.0, 300.0};

    test_utilities::test_double_equality_of_components(tResult, tGold,
                                                       TEST_CONTEXT("Coordinate coordinate multiplication"));
}

TEST(Vector3, CoordinateFloor)
{
    constexpr Coordinate tInput{2.2, 5.7, 1.3};
    const Coordinate tResult = floor(tInput);
    constexpr Coordinate tGold{2.0, 5.0, 1.0};

    test_utilities::test_double_equality_of_components(tResult, tGold, TEST_CONTEXT("Coordinate floor"));
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
        test_utilities::test_double_equality_of_components(tResult, kZ, TEST_CONTEXT("Vector3 cross product"));
    }
    {
        constexpr Vector3 tResult = cross(kY, k123);
        constexpr Vector3 tGold{3, 0, -1};
        test_utilities::test_double_equality_of_components(tResult, tGold, TEST_CONTEXT("Vector3 cross product"));
    }
    {
        constexpr Vector3 tResult = cross(k123, k123);
        constexpr Vector3 tGold{0, 0, 0};
        test_utilities::test_double_equality_of_components(tResult, tGold, TEST_CONTEXT("Vector3 cross product"));
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

namespace
{
void test_unit_vector(const UnitVector3& aUnitVec)
{
    const double tMag = std::sqrt(1.0 + 4.0 + 9.0);
    const Vector3 tGold{1.0 / tMag, 2.0 / tMag, 3.0 / tMag};
    EXPECT_EQ(tGold.x, aUnitVec.x);
    EXPECT_EQ(tGold.y, aUnitVec.y);
    EXPECT_EQ(tGold.z, aUnitVec.z);
}
}  // namespace

TEST(UnitVector3, ComponentConstructor)
{
    const UnitVector3 tUnitVec3(1, 2, 3);
    test_unit_vector(tUnitVec3);
}

TEST(UnitVector3, Vector3Constructor)
{
    constexpr Vector3 tVector{1, 2, 3};
    const UnitVector3 tUnitVec3(tVector);
    test_unit_vector(tUnitVec3);
}

TEST(Coordinate, EqualityOperator)
{
    constexpr auto tCoordinateAll1 = Coordinate{1.0, 1.0, 1.0};
    constexpr auto tCoordinate1x = Coordinate{1.0, 0.0, 0.0};
    constexpr auto tCoordinate1y = Coordinate{0.0, 1.0, 0.0};
    constexpr auto tCoordinate1z = Coordinate{0.0, 0.0, 1.0};

    EXPECT_TRUE(tCoordinateAll1 == tCoordinateAll1);
    EXPECT_FALSE(tCoordinateAll1 == tCoordinate1x);
    EXPECT_FALSE(tCoordinateAll1 == tCoordinate1y);
    EXPECT_FALSE(tCoordinateAll1 == tCoordinate1z);

    // Reverse order
    EXPECT_FALSE(tCoordinate1x == tCoordinateAll1);
    EXPECT_FALSE(tCoordinate1y == tCoordinateAll1);
    EXPECT_FALSE(tCoordinate1z == tCoordinateAll1);

    // x with others
    EXPECT_TRUE(tCoordinate1x == tCoordinate1x);
    EXPECT_FALSE(tCoordinate1x == tCoordinate1y);
    EXPECT_FALSE(tCoordinate1x == tCoordinate1z);
    EXPECT_FALSE(tCoordinate1y == tCoordinate1x);
    EXPECT_FALSE(tCoordinate1z == tCoordinate1x);

    // y with others
    EXPECT_TRUE(tCoordinate1y == tCoordinate1y);
    EXPECT_FALSE(tCoordinate1y == tCoordinate1z);
    EXPECT_FALSE(tCoordinate1z == tCoordinate1y);

    // z with itself
    EXPECT_TRUE(tCoordinate1z == tCoordinate1z);
}

}  // namespace plato::third_party_integration::common::unittest
