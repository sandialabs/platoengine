#include <gtest/gtest.h>

#include "plato/core/ValidationUtilities.hpp"
#include "plato/input_validation/ValidationRegistration.hpp"
#include "plato/process_manager/extension/GradientCheck.hpp"
#include "plato/process_manager/library/ValidatedInput.hpp"
#include "plato/test_utilities/InputGeneration.hpp"
#include "plato/utilities/Exception.hpp"

namespace plato::process_manager::extension::unittest
{
TEST(ValidateGradientCheck, ValidateOuputFileName)
{
    auto tGradientCheck = input_parser::new_gradient_check{};
    EXPECT_TRUE(detail::validate_output_file_name(tGradientCheck).has_value());
    tGradientCheck.output_file_name = input_parser::FileName{"file.txt"};  // Requires an input
    EXPECT_FALSE(detail::validate_output_file_name(tGradientCheck).has_value());
}

TEST(ValidateGradientCheck, NoErrorMessagesValidGradientCheck)
{
    const auto tGradientCheck = create_valid_example_gradient_check();
    const auto tMessages = input_validation::validate(tGradientCheck, {});
    EXPECT_TRUE(tMessages.empty());
}

TEST(ValidateGradientCheck, ErrorMessagesInvalidGradientCheck)
{
    const auto tGradientCheck = input_parser::new_gradient_check{};
    const auto tMessages = input_validation::validate(tGradientCheck, {});
    const auto tNumberOfGradientCheckValidationFunctions =
        input_validation::detail::registered_validation_functions<input_parser::new_gradient_check>().size();
    EXPECT_EQ(tMessages.size(), tNumberOfGradientCheckValidationFunctions);
}

}  // namespace plato::process_manager::extension::unittest
