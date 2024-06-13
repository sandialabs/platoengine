#include <gtest/gtest.h>

#include "plato/core/ValidationRegistration.hpp"
#include "plato/input_parser/InputBlockUtilities.hpp"
#include "plato/process_manager/library/ProcessManagerValidation.hpp"
#include "plato/test_utilities/InputGeneration.hpp"

namespace plato::process_manager::library::unittest
{
namespace
{
}
TEST(ProcessManagerValidation, InValidParsedInputNoProcessManagers)
{
    const auto tInput = input_parser::ParsedInput{};
    EXPECT_TRUE(detail::validate_at_least_one_process_manager(tInput).has_value());
}

TEST(ProcessManagerValidation, NoErrorMessagesValidGradientCheck)
{
    const input_parser::ParsedInput tInput = test_utilities::create_valid_example_objective() |
                                             test_utilities::create_valid_brick_shape_geometry() |
                                             test_utilities::create_valid_example_gradient_check();
    const auto tMessages = validate_process_managers(tInput, std::vector<std::string>{});
    EXPECT_TRUE(tMessages.empty());
}

TEST(ProcessManagerValidation, ErrorMessagesForInvalidGradientCheck)
{
    input_parser::ParsedInput tInput = test_utilities::create_valid_example_objective() |
                                       test_utilities::create_valid_brick_shape_geometry() |
                                       test_utilities::create_valid_example_gradient_check();
    EXPECT_TRUE(validate_process_managers(tInput, std::vector<std::string>{}).empty());

    // Make input invalid
    tInput.mGradientCheck->random_direction_seed = 0;
    EXPECT_EQ(validate_process_managers(tInput, std::vector<std::string>{}).size(), 1u);
}
}  // namespace plato::process_manager::library::unittest
