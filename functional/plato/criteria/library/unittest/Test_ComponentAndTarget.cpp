#include <gtest/gtest.h>

#include "plato/criteria/library/ComponentAndTarget.hpp"
#include "plato/input_parser/test_utilities/SequenceSubtypeExpectHelpers.hpp"

namespace plato::criteria::library::unittest
{
TEST(ComponentAndTarget, Valid)
{
    constexpr auto tInput = std::string_view{"component mass target 42.0"};
    const auto tExpected =
        input_parser::ComponentAndTarget{/*.component=*/input_parser::IdentifierString{"mass"}, /*.target=*/42.0};
    input_parser::test_utilities::expect_valid_input<input_parser::ComponentAndTarget>(tInput, tExpected,
                                                                                       TEST_CONTEXT("Valid input"));
}

TEST(ComponentAndTarget, Invalid)
{
    {
        const auto tInput = std::string_view{"component cg_x "};
        input_parser::test_utilities::expect_invalid_input<input_parser::ComponentAndTarget>(
            tInput, TEST_CONTEXT("Invalid missing target"));
    }
    {
        const auto tInput = std::string_view{"target 42.0"};
        input_parser::test_utilities::expect_invalid_input<input_parser::ComponentAndTarget>(
            tInput, TEST_CONTEXT("Invalid missing component name"));
    }
    {
        const auto tInput = std::string_view{"target 42.0 component cg_z"};
        input_parser::test_utilities::expect_invalid_input<input_parser::ComponentAndTarget>(
            tInput, TEST_CONTEXT("Invalid out of order"));
    }
}

}  // namespace plato::criteria::library::unittest
