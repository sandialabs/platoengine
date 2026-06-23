#include <gtest/gtest.h>

#include "plato/criteria/library/ComponentAndTarget.hpp"
#include "plato/input_parser/AutoList.hpp"
#include "plato/input_parser/InputFieldTypes.hpp"
#include "plato/input_parser/test_utilities/TestHelpers.hpp"
#include "plato/test_utilities/TestContext.hpp"

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

TEST(ComponentAndTarget, DataTypeNames)
{
    EXPECT_EQ(data_type_documentation(input_parser::ComponentAndTarget{}),
              "Composite type: `component` {String} `target` {Floating point} ");
    EXPECT_EQ(data_type_documentation(input_parser::AutoList<input_parser::ComponentAndTarget>{}),
              "Comma separated list of: Composite type: `component` {String} `target` {Floating point} ");
}

}  // namespace plato::criteria::library::unittest
