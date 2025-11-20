#include <gtest/gtest.h>

#include <string_view>

#include "plato/input_parser/Range.hpp"
#include "plato/input_parser/test_utilities/TestHelpers.hpp"
#include "plato/test_utilities/TestContext.hpp"

namespace plato::input_parser::unittest
{

TEST(Range, ParsingTests)
{
    {
        const auto tInput = std::string_view{" [1.0:1.0:2.0] "};
        const auto tGold = input_parser::Range{1.0, 1.0, 2.0};
        input_parser::test_utilities::expect_valid_input(tInput, tGold, TEST_CONTEXT("Simple valid range"));
    }
    {
        const auto tInput = std::string_view{"1.0e-1:1.0e-2:2.0]"};
        input_parser::test_utilities::expect_invalid_input<input_parser::Range>(
            tInput, TEST_CONTEXT("Invalid floating point bounds missing first ["));
    }
    {
        const auto tInput = std::string_view{"[1.0:1.0:2.0"};
        input_parser::test_utilities::expect_invalid_input<input_parser::Range>(
            tInput, TEST_CONTEXT("Invalid floating point missing ending ]"));
    }
    {
        const auto tInput = std::string_view{"[1:3]"};
        input_parser::test_utilities::expect_invalid_input<input_parser::Range>(
            tInput, TEST_CONTEXT("Invalid too few entries"));
    }
}

}  // namespace plato::input_parser::unittest
