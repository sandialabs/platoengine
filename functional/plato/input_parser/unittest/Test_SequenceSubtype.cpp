#include <gtest/gtest.h>

#include "plato/input_parser/SequenceSubtype.hpp"
#include "plato/input_parser/test_utilities/TestHelpers.hpp"
#include "plato/test_utilities/TestContext.hpp"

// clang-format off
PLATO_INPUT_SEQUENCE_SUBTYPE(
(plato)(input_parser),SNLEmployee,
(int, age)
(bool, meets_expectations)
(double, salary)
)
// clang-format on

namespace plato::input_parser::unittest
{

TEST(SequenceSubtype, ParsingTests)
{
    {
        const auto tInput = std::string_view{"age 55 meets_expectations false salary 100000000.0 "};
        const auto tGold = SNLEmployee{55, false, 100000000};
        test_utilities::expect_valid_input<SNLEmployee>(tInput, tGold, TEST_CONTEXT("Absolutely nothing wrong here."));
    }
    {
        const auto tInput = std::string_view{"age 42.5 meets_expectations true salary 100.0 "};
        test_utilities::expect_invalid_input<SNLEmployee>(tInput, TEST_CONTEXT("You gotta be kidding me."));
    }
    {
        const auto tInput = std::string_view{" meets_expectations true age 1 salary 3.0 "};
        test_utilities::expect_invalid_input<SNLEmployee>(tInput, TEST_CONTEXT("Out of order input."));
    }
    {
        const auto tInput = std::string_view{" age 1 salary 3.0 "};
        test_utilities::expect_invalid_input<SNLEmployee>(tInput, TEST_CONTEXT("Missing all required entries."));
    }
}

}  // namespace plato::input_parser::unittest
