#include <gtest/gtest.h>

#include <string_view>

#include "plato/input_parser/Bounds.hpp"
#include "plato/input_parser/test_utilities/TestHelpers.hpp"
#include "plato/test_utilities/TestContext.hpp"

namespace plato::input_parser::unittest
{

TEST(Bounds, ParsingTests)
{
    {
        const auto tInput = std::string_view{" [1.0, 2.0] "};
        const auto tGold = input_parser::Bounds(1.0, 2.0);
        input_parser::test_utilities::expect_valid_input(tInput, tGold, TEST_CONTEXT("Simple valid bounds"));
    }
    {
        const auto tInput = std::string_view{"4.0e3, 5.0E3]"};
        input_parser::test_utilities::expect_invalid_input<input_parser::Bounds>(
            tInput, TEST_CONTEXT("Invalid floating point bounds missing first ["));
    }
    {
        const auto tInput = std::string_view{"[6.0e-8, -7.0E9"};
        input_parser::test_utilities::expect_invalid_input<input_parser::Bounds>(
            tInput, TEST_CONTEXT("Invalid floating point missing ending ]"));
    }
    {
        const auto tInput = std::string_view{"[1,2,3]"};
        input_parser::test_utilities::expect_invalid_input<input_parser::Bounds>(
            tInput, TEST_CONTEXT("Invalid too many entries"));
    }
}

}  // namespace plato::input_parser::unittest
