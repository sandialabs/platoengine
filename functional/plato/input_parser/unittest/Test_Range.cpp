#include <gtest/gtest.h>

#include <string_view>

#include "plato/input_parser/Range.hpp"
#include "plato/input_parser/test_utilities/TestHelpers.hpp"
#include "plato/test_utilities/TestContext.hpp"

namespace plato::input_parser::unittest
{

namespace
{

void expect_valid_input(const std::string_view aInput,
                        const input_parser::Range aGold,
                        const plato::test_utilities::TestContext& aTestContext)
{
    const auto [tResult, tSuccess] = test_utilities::parse_input<Range>(aInput);
    ASSERT_TRUE(tSuccess) << aTestContext;
    EXPECT_EQ(tResult.mLower, aGold.mLower) << aTestContext;
    EXPECT_EQ(tResult.mStep, aGold.mStep) << aTestContext;
    EXPECT_EQ(tResult.mUpper, aGold.mUpper) << aTestContext;
}

void expect_invalid_input(const std::string_view aInput, const plato::test_utilities::TestContext& aTestContext)
{
    const auto [tResult, tSuccess] = test_utilities::parse_input<Range>(aInput);
    EXPECT_FALSE(tSuccess) << aTestContext;
}

}  // namespace

TEST(Range, ParsingTests)
{
    {
        const auto tInput = std::string_view{" [1.0:1.0:2.0] "};
        const auto tGold = input_parser::Range{1.0, 1.0, 2.0};
        expect_valid_input(tInput, tGold, TEST_CONTEXT("Simple valid range"));
    }
    {
        const auto tInput = std::string_view{"1.0e-1:1.0e-2:2.0]"};
        expect_invalid_input(tInput, TEST_CONTEXT("Invalid floating point bounds missing first ["));
    }
    {
        const auto tInput = std::string_view{"[1.0:1.0:2.0"};
        expect_invalid_input(tInput, TEST_CONTEXT("Invalid floating point missing ending ]"));
    }
    {
        const auto tInput = std::string_view{"[1:3]"};
        expect_invalid_input(tInput, TEST_CONTEXT("Invalid too few entries"));
    }
}

}  // namespace plato::input_parser::unittest
