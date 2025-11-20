#include <gtest/gtest.h>

#include "plato/geometry/extension/LevelSetTopologySphereParser.hpp"
#include "plato/input_parser/test_utilities/TestHelpers.hpp"
#include "plato/test_utilities/TestContext.hpp"

namespace plato::geometry::extension::cubit::unittest
{

TEST(LevelSetSphere, ValidInputParsingTest)
{
    constexpr auto tValidInput = std::string_view{" radius 2.0 center (0.1,0.2,0.3)"};
    const auto tGold = input_parser::LevelSetSphere{/*.radius=*/2.0, input_parser::Point{0.1, 0.2, 0.3}};
    input_parser::test_utilities::expect_valid_input(tValidInput, tGold, TEST_CONTEXT("Valid input"));
}

TEST(LevelSetSphere, InvalidInputParsingTest)
{
    {
        constexpr auto tInvalidInput = std::string_view{"center (0.1,0.2,0.3) radius 1"};
        input_parser::test_utilities::expect_invalid_input<input_parser::LevelSetSphere>(tInvalidInput,
                                                                                         TEST_CONTEXT("Wrong order"));
    }
    {
        constexpr auto tInvalidInput = std::string_view{"center (0.1,0.2,0.3) "};
        input_parser::test_utilities::expect_invalid_input<input_parser::LevelSetSphere>(
            tInvalidInput, TEST_CONTEXT("Missing radius"));
    }
    {
        constexpr auto tInvalidInput = std::string_view{"radius 2.0 "};
        input_parser::test_utilities::expect_invalid_input<input_parser::LevelSetSphere>(
            tInvalidInput, TEST_CONTEXT("Missing center"));
    }
}

TEST(LevelSetSphere, ValidInputParsingTestTwoEntries)
{
    constexpr auto tValidInput = std::string_view{
        "radius 0.5 center (-1,-2,-3) ,"
        "radius 1 center (1,2,3)  \n "};
    const auto [tResult, tSuccess] =
        input_parser::test_utilities::parse_input<input_parser::LevelSetSphereList>(tValidInput);

    const auto tGoldFront = input_parser::LevelSetSphere{/*.radius=*/0.5, input_parser::Point{-1, -2, -3}};
    const auto tGoldBack = input_parser::LevelSetSphere{/*.radius=*/1, input_parser::Point{1, 2, 3}};
    ASSERT_TRUE(tSuccess) << "Valid input";
    ASSERT_EQ(tResult.mList.size(), 2U) << "Parsing two items";

    EXPECT_EQ(tResult.mList.front(), tGoldFront) << TEST_CONTEXT("Valid input front result");
    EXPECT_EQ(tResult.mList.back(), tGoldBack) << TEST_CONTEXT("Valid input back result");
}

TEST(LevelSetSphereListParser, ValidInputParsingTestEmptyList)
{
    constexpr auto tValidInput = std::string_view{""};
    const auto [tResult, tSuccess] =
        input_parser::test_utilities::parse_input<input_parser::LevelSetSphereList>(tValidInput);
    EXPECT_FALSE(tSuccess) << "Invalid input, empty list";
}

TEST(LevelSetSphereListParser, ValidInputParsingTestSingleEntry)
{
    constexpr auto tValidInput = std::string_view{"radius 0.5 center (-1,-2,-3)   "};
    const auto [tResult, tSuccess] =
        input_parser::test_utilities::parse_input<input_parser::LevelSetSphereList>(tValidInput);

    ASSERT_TRUE(tSuccess) << "Valid input";
    ASSERT_EQ(tResult.mList.size(), 1U);

    const auto tGold = input_parser::LevelSetSphere{/*.radius=*/0.5, input_parser::Point{-1, -2, -3}};
    EXPECT_EQ(tResult.mList.front(), tGold) << TEST_CONTEXT("Valid input single entry");
}

}  // namespace plato::geometry::extension::cubit::unittest
