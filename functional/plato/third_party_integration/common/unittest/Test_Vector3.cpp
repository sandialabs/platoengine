#include <gtest/gtest.h>

#include <cmath>
#include <numbers>
#include <sstream>
#include <stdexcept>
#include <vector>

#include "plato/test_utilities/TestContext.hpp"
#include "plato/third_party_integration/common/Vector3.hpp"
#include "plato/third_party_integration/common/test_utilities/CoordinateTestUtilities.hpp"

namespace plato::third_party_integration::common::unittest
{
namespace
{
constexpr Vector3 kUnitX{1, 0, 0};
constexpr Vector3 kUnitY{0, 1, 0};
constexpr Vector3 kUnitZ{0, 0, 1};
constexpr Vector3 k123{1, 2, 3};
}  // namespace

TEST(Vector3, CoordinateSubtraction)
{
    constexpr Coordinate tP1{1, 2, 3};
    constexpr Coordinate tP2{4, 5, 6};
    constexpr Vector3 tResult = tP2 - tP1;
    constexpr Vector3 tGold{3, 3, 3};

    test_utilities::test_double_equality_of_components(tResult, tGold, TEST_CONTEXT("Vector3 subtraction"));
}

TEST(Vector3, CoordinateAddition)
{
    constexpr Coordinate tP1{38, 21, 86};
    constexpr Coordinate tP2{71, 91, 10};
    {
        constexpr Coordinate tResult = tP1 + tP2;
        constexpr Coordinate tGold{109, 112, 96};
        test_utilities::test_double_equality_of_components(tResult, tGold, TEST_CONTEXT("Coordinate addition"));
    }

    {
        constexpr Coordinate tResult = tP1 + tP2 + tP1;
        constexpr Coordinate tGold{147, 133, 182};
        test_utilities::test_double_equality_of_components(tResult, tGold, TEST_CONTEXT("Coordinate addition"));
    }
}

TEST(Vector3, CoordinateScalarMultiplication)
{
    constexpr Coordinate tP1{1, 2, 3};
    constexpr double tScale{10};
    constexpr Coordinate tResult = tP1 * tScale;
    constexpr Coordinate tGold{10, 20, 30};

    test_utilities::test_double_equality_of_components(tResult, tGold, TEST_CONTEXT("Coordinate multiplication"));
}

TEST(Vector3, CoordinateScalarDivision)
{
    constexpr Coordinate tP1{10, 20, 30};
    constexpr double tDivisor{10};
    constexpr Coordinate tResult = tP1 / tDivisor;
    constexpr Coordinate tGold{1, 2, 3};

    test_utilities::test_double_equality_of_components(tResult, tGold, TEST_CONTEXT("Vector3 scalar division"));
}

TEST(Vector3, CoordinateScalarAddition)
{
    constexpr Coordinate tP1{10, 20, 30};
    constexpr double tScalar{10};
    constexpr Coordinate tResult = tP1 + tScalar;
    constexpr Coordinate tGold{20, 30, 40};

    test_utilities::test_double_equality_of_components(tResult, tGold, TEST_CONTEXT("Vector3 scalar addition"));
}

TEST(Coordinate, CoordinateCoordinateDivision)
{
    constexpr Coordinate tP1{10, 20, 30};
    constexpr Coordinate tDivisor{2.0, 4.0, 10.0};
    constexpr Coordinate tResult = tP1 / tDivisor;
    constexpr Coordinate tGold{5.0, 5.0, 3.0};

    test_utilities::test_double_equality_of_components(tResult, tGold, TEST_CONTEXT("Coordinate coordinate division"));
}

TEST(Vector3, ScalarCoordinateDivision)
{
    constexpr double tScalar{10};
    constexpr Coordinate tDivisor{2.0, 5.0, 1.0};
    constexpr Coordinate tResult = tScalar / tDivisor;
    constexpr Coordinate tGold{5.0, 2.0, 10.0};

    test_utilities::test_double_equality_of_components(tResult, tGold, TEST_CONTEXT("Scalar coordinate division"));
}

TEST(Vector3, CoordinateCoordinateMultiply)
{
    constexpr Coordinate tP1{10, 20, 30};
    constexpr Coordinate tMultiplier{2.0, 4.0, 10.0};
    constexpr Coordinate tResult = tP1 * tMultiplier;
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

TEST(Coordinate, OutstreamOperator)
{
    const auto tCoordinate = Coordinate{1, 2, 3};
    std::stringstream tStringStream;
    tStringStream << tCoordinate;
    const auto tString = tStringStream.str();
    const auto tGoldString = std::string{"1\t2\t3"};
    EXPECT_EQ(tString, tGoldString);
}

TEST(Coordinate, String)
{
    const auto tCoordinate = Coordinate{1.123456789, 2.123456789, 3.123456789};
    const auto tPrecision = 4U;
    const auto tString = string(tCoordinate, tPrecision);
    const auto tGold = std::string{"(1.1235, 2.1235, 3.1235)"};
    EXPECT_EQ(tString, tGold);
}

TEST(Vector3, Dot)
{
    {
        constexpr double tResult = dot(kUnitX, kUnitY);
        constexpr double tGold = 0.0;
        EXPECT_DOUBLE_EQ(tResult, tGold);
    }
    {
        constexpr double tResult = dot(kUnitZ, k123);
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
        constexpr Vector3 tResult = cross(kUnitX, kUnitY);
        test_utilities::test_double_equality_of_components(tResult, kUnitZ, TEST_CONTEXT("Vector3 cross product"));
    }
    {
        constexpr Vector3 tResult = cross(kUnitY, k123);
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
    constexpr auto tCoordinate = Coordinate{-1, -2, -3};
    const auto tGold = std::sqrt(1 + 4 + 9);
    EXPECT_EQ(tGold, magnitude(tCoordinate));
}

TEST(Vector3, Magnitude)
{
    constexpr auto tVector = Vector3{1, 2, 3};
    const auto tGold = std::sqrt(1 + 4 + 9);
    EXPECT_EQ(tGold, magnitude(tVector));
}

TEST(Vector3, Normalize)
{
    auto tVector = Vector3{1, 2, 4};
    const auto tGold = tVector / magnitude(tVector);
    normalize(tVector);
    EXPECT_EQ(tGold, tVector);
}

namespace
{
void test_unit_vector(const UnitVector3& aUnitVector, const plato::test_utilities::TestContext& aTestContext)
{
    const auto tMagnitude = std::sqrt(1.0 + 4.0 + 9.0);
    const auto tGold = Vector3{1.0 / tMagnitude, 2.0 / tMagnitude, 3.0 / tMagnitude};
    EXPECT_DOUBLE_EQ(tGold.x, static_cast<Vector3>(aUnitVector).x) << aTestContext;
    EXPECT_DOUBLE_EQ(tGold.y, static_cast<Vector3>(aUnitVector).y) << aTestContext;
    EXPECT_DOUBLE_EQ(tGold.z, static_cast<Vector3>(aUnitVector).z) << aTestContext;
}
}  // namespace

TEST(UnitVector3, ComponentConstructor)
{
    const auto tUnitVector3 = UnitVector3(1, 2, 3);
    test_unit_vector(tUnitVector3, TEST_CONTEXT("Testing construction of unit vector with x, y, z, components."));
}

TEST(UnitVector3, Vector3Constructor)
{
    constexpr auto tVector = Vector3{1, 2, 3};
    const auto tUnitVector3 = UnitVector3(tVector);
    test_unit_vector(tUnitVector3, TEST_CONTEXT("Testing construction of unit vector with non unit vector."));
}

TEST(UnitVector3, ErrorZeroLengthVector)
{
    constexpr auto tVector = Vector3{0, 0, 0};
    EXPECT_THROW([[maybe_unused]] const UnitVector3 tUnitVector{tVector}, std::invalid_argument);
}

TEST(UnitVector3, Vector3ConversionOperator)
{
    constexpr auto tComponent{1.0};
    const auto tMagnitude = std::numbers::sqrt3;
    const auto tNormalizedComponent = tComponent / tMagnitude;
    const UnitVector3 tUnitVector{tComponent, tComponent, tComponent};
    EXPECT_DOUBLE_EQ(static_cast<Vector3>(tUnitVector).x, tNormalizedComponent);
    EXPECT_DOUBLE_EQ(static_cast<Vector3>(tUnitVector).y, tNormalizedComponent);
    EXPECT_DOUBLE_EQ(static_cast<Vector3>(tUnitVector).z, tNormalizedComponent);
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

TEST(Vector3, MinimimEntries)
{
    const auto tVectorOne = Vector3{-1, 2, -3};
    const auto tVectorTwo = Vector3{4, -5, 6};

    const auto tResult = minimum_entries(tVectorOne, tVectorTwo);
    const auto tGold = Vector3{-1, -5, -3};
    EXPECT_EQ(tGold, tResult);
}

TEST(Coordinate, MaximimEntries)
{
    const auto tCoordinateOne = Coordinate{-1, 2, -3};
    const auto tCoordinateTwo = Coordinate{4, -5, 6};

    const auto tResult = maximum_entries(tCoordinateOne, tCoordinateTwo);
    const auto tGold = Coordinate{4, 2, 6};
    EXPECT_EQ(tGold, tResult);
}

TEST(Coordinate, Flatten)
{
    const auto tVector = std::vector<Coordinate>{{1, 2, 3}, {4, 5, 6}};
    const auto tGoldVector = std::vector<double>{1, 2, 3, 4, 5, 6};
    const auto tResult = flatten(tVector);
    EXPECT_EQ(tResult, tGoldVector);
}

}  // namespace plato::third_party_integration::common::unittest
