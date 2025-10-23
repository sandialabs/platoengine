#include <gtest/gtest.h>

#include "plato/geometry/extension/cubit/BoundedApreproVariableParser.hpp"
#include "plato/input_parser/test_utilities/TestHelpers.hpp"
#include "plato/test_utilities/TestContext.hpp"

namespace plato::geometry::extension::cubit::unittest
{
namespace
{

void expect_equal(const input_parser::BoundedApreproVariable& aResult,
                  const input_parser::BoundedApreproVariable& aGold,
                  const test_utilities::TestContext& aTestContext)
{
    EXPECT_EQ(aResult.mName.mToken, aGold.mName.mToken) << aTestContext;
    EXPECT_EQ(aResult.mInitialValue, aGold.mInitialValue) << aTestContext;
    EXPECT_EQ(aResult.mBounds.mLower, aGold.mBounds.mLower) << aTestContext;
    EXPECT_EQ(aResult.mBounds.mUpper, aGold.mBounds.mUpper) << aTestContext;
}

void expect_invalid_input(const std::string_view aInput, const plato::test_utilities::TestContext& aTestContext)
{
    const auto [tResult, tSuccess] =
        input_parser::test_utilities::parse_input<input_parser::BoundedApreproVariable>(aInput);
    ASSERT_FALSE(tSuccess) << aTestContext;
}

}  // namespace

TEST(BoundedApreproVariable, ValidInputParsingTest)
{
    constexpr auto tValidInput = std::string_view{"width 0.5 [0.2, 1.0]"};
    const auto [tResult, tSuccess] =
        input_parser::test_utilities::parse_input<input_parser::BoundedApreproVariable>(tValidInput);

    const auto tGold =
        input_parser::BoundedApreproVariable{input_parser::ApreproString{"width"}, 0.5, input_parser::Bounds{0.2, 1.0}};
    ASSERT_TRUE(tSuccess) << "Valid input";
    expect_equal(tResult, tGold, TEST_CONTEXT("Valid input"));
}

TEST(BoundedApreproVariable, InvalidInputParsingTest)
{
    {
        constexpr auto tInvalidInput = std::string_view{"width, 0.5 [0.2, 1.0]"};
        expect_invalid_input(tInvalidInput, TEST_CONTEXT("Erroneous comma"));
    }
    {
        constexpr auto tInvalidInput = std::string_view{"width 0.5 "};
        expect_invalid_input(tInvalidInput, TEST_CONTEXT("Missing bounds"));
    }
    {
        constexpr auto tInvalidInput = std::string_view{"#width, 0.5 [0.2, 1.0]"};
        expect_invalid_input(tInvalidInput, TEST_CONTEXT("Misplaced comment"));
    }
    {
        constexpr auto tInvalidInput = std::string_view{" a-b  0.5 [0.2, 1.0]"};
        expect_invalid_input(tInvalidInput, TEST_CONTEXT("Missing identifier"));
    }
    {
        constexpr auto tInvalidInput = std::string_view{"'width'  0.5 [0.2, 1.0]"};
        expect_invalid_input(tInvalidInput, TEST_CONTEXT("Old syntax starter key"));
    }
}

TEST(BoundedApreproVariable, ValidInputParsingTestTwoEntries)
{
    constexpr auto tValidInput = std::string_view{
        " width 0.5 [0.2, 1.0],"
        " height 1 [0.5, 2.0]\n "};
    const auto [tResult, tSuccess] =
        input_parser::test_utilities::parse_input<input_parser::BoundedApreproVariableList>(tValidInput);

    const auto tGoldFront =
        input_parser::BoundedApreproVariable{input_parser::ApreproString{"width"}, 0.5, input_parser::Bounds{0.2, 1.0}};
    const auto tGoldBack =
        input_parser::BoundedApreproVariable{input_parser::ApreproString{"height"}, 1, input_parser::Bounds{0.5, 2.0}};
    ASSERT_TRUE(tSuccess) << "Valid input";
    ASSERT_EQ(tResult.mList.size(), 2U) << "Parsing two items";

    expect_equal(tResult.mList.front(), tGoldFront, TEST_CONTEXT("Valid input front result"));
    expect_equal(tResult.mList.back(), tGoldBack, TEST_CONTEXT("Valid input back result"));
}

TEST(BoundedApreproVariableListParser, ValidInputParsingTestEmptyList)
{
    constexpr auto tValidInput = std::string_view{""};
    const auto [tResult, tSuccess] =
        input_parser::test_utilities::parse_input<input_parser::BoundedApreproVariableList>(tValidInput);
    EXPECT_FALSE(tSuccess) << "Invalid input, empty list";
}

TEST(BoundedApreproVariableListParser, ValidInputParsingTestSingleEntry)
{
    constexpr auto tValidInput = std::string_view{" width 0.5 [0.2, 1.0] "};
    const auto [tResult, tSuccess] =
        input_parser::test_utilities::parse_input<input_parser::BoundedApreproVariableList>(tValidInput);

    ASSERT_TRUE(tSuccess) << "Valid input";
    ASSERT_EQ(tResult.mList.size(), 1U);

    const auto tGold =
        input_parser::BoundedApreproVariable{input_parser::ApreproString{"width"}, 0.5, input_parser::Bounds{0.2, 1.0}};
    expect_equal(tResult.mList.front(), tGold, TEST_CONTEXT("Valid input single entry"));
}

}  // namespace plato::geometry::extension::cubit::unittest
