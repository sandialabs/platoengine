#include <gtest/gtest.h>

#include <string_view>

#include "plato/input_parser/Bounds.hpp"
#include "plato/input_parser/unittest/Test_Helpers.hpp"
#include "plato/test_utilities/TestContext.hpp"

namespace plato::input_parser::unittest
{

namespace
{

void expect_valid_input(const std::string_view aInput,
                        const std::pair<double, double> aGold,
                        const test_utilities::TestContext& aTestContext)
{
    const auto [tResult, tSuccess] = unittest::parse_input<Bounds>(aInput);
    ASSERT_TRUE(tSuccess) << aTestContext;
    EXPECT_EQ(tResult.mLower, aGold.first) << aTestContext;
    EXPECT_EQ(tResult.mUpper, aGold.second) << aTestContext;
}

void expect_invalid_input(const std::string_view aInput, const test_utilities::TestContext& aTestContext)
{
    const auto [tResult, tSuccess] = unittest::parse_input<Bounds>(aInput);
    EXPECT_FALSE(tSuccess) << aTestContext;
}

}  // namespace

TEST(Bounds, ParsingTests)
{
    {
        const auto tInput = std::string_view{" [1.0, 2.0] "};
        const auto tGold = std::make_pair(1.0, 2.0);
        expect_valid_input(tInput, tGold, TEST_CONTEXT("Simple valid bounds"));
    }
    {
        const auto tInput = std::string_view{"4.0e3, 5.0E3]"};
        expect_invalid_input(tInput, TEST_CONTEXT("Invalid floating point bounds missing first ["));
    }
    {
        const auto tInput = std::string_view{"[6.0e-8, -7.0E9"};
        expect_invalid_input(tInput, TEST_CONTEXT("Invalid floating point missing ending ]"));
    }
    {
        const auto tInput = std::string_view{"[1,2,3]"};
        expect_invalid_input(tInput, TEST_CONTEXT("Invalid too many entries"));
    }
}

}  // namespace plato::input_parser::unittest
