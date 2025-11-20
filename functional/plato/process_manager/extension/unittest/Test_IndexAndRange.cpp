#include <gtest/gtest.h>

#include <string_view>

#include "plato/input_parser/test_utilities/TestHelpers.hpp"
#include "plato/process_manager/extension/IndexAndRange.hpp"
#include "plato/test_utilities/TestContext.hpp"

namespace plato::process_manager::extension::unittest
{

TEST(IndexAndRange, ParsingTests)
{
    {
        const auto tInput = std::string_view{"index 1 range [1.0:1.0:2.0] "};
        const auto tGold = input_parser::IndexAndRange{1, input_parser::Range{1.0, 1.0, 2.0}};
        input_parser::test_utilities::expect_valid_input<input_parser::IndexAndRange>(tInput, tGold,
                                                                                      TEST_CONTEXT("Valid range"));
    }
    {
        const auto tInput = std::string_view{"index 1 "};
        input_parser::test_utilities::expect_invalid_input<input_parser::IndexAndRange>(
            tInput, TEST_CONTEXT("Invalid missing range"));
    }
    {
        const auto tInput = std::string_view{"index 1 []"};
        input_parser::test_utilities::expect_invalid_input<input_parser::IndexAndRange>(
            tInput, TEST_CONTEXT("Invalid missing range"));
    }
    {
        const auto tInput = std::string_view{"range [0:1:10]"};
        input_parser::test_utilities::expect_invalid_input<input_parser::IndexAndRange>(
            tInput, TEST_CONTEXT("Invalid missing index"));
    }
}

}  // namespace plato::process_manager::extension::unittest
