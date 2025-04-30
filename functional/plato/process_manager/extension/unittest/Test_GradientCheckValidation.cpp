#include <gtest/gtest.h>

#include "plato/criteria/library/test_utilities/ExampleInputBlocks.hpp"
#include "plato/geometry/extension/test_utilities/ExampleInputBlocks.hpp"
#include "plato/input_parser/InputBlockUtilities.hpp"
#include "plato/input_validation/ValidationRegistration.hpp"
#include "plato/input_validation/ValidationUtilities.hpp"
#include "plato/process_manager/extension/GradientCheck.hpp"
#include "plato/process_manager/extension/test_utilities/ExampleInputBlocks.hpp"
#include "plato/utilities/Exception.hpp"

namespace plato::process_manager::extension::unittest
{
TEST(ValidateGradientCheck, ValidateOuputFileName)
{
    auto tGradientCheck = input_parser::gradient_check{};
    EXPECT_TRUE(detail::validate_output_file_name(tGradientCheck).has_value());
    tGradientCheck.output_file_name = input_parser::FileName{"file.txt"};  // Requires an input
    EXPECT_FALSE(detail::validate_output_file_name(tGradientCheck).has_value());
}

TEST(ValidateGradientCheck, NoErrorMessagesValidGradientCheck)
{
    const auto tGradientCheck = test_utilities::create_valid_example_gradient_check_input();
    const auto tMessages = input_validation::validate(tGradientCheck, {});
    EXPECT_TRUE(tMessages.empty());
}

TEST(ValidateGradientCheck, ErrorMessagesInvalidGradientCheck)
{
    const auto tGradientCheck = input_parser::gradient_check{};
    const auto tMessages = input_validation::validate(tGradientCheck, {});
    const auto tNumberOfGradientCheckValidationFunctions =
        input_validation::detail::registered_validation_functions<input_parser::gradient_check>().size();
    EXPECT_EQ(tMessages.size(), tNumberOfGradientCheckValidationFunctions);
}

TEST(ValidateGradientCheck, RandomDirectionSeedViaRegistration)
{
    const auto tValidInputBase = criteria::library::test_utilities::create_valid_example_objective_input() |
                                 geometry::extension::test_utilities::create_valid_brick_shape_geometry_input();

    const auto tValidInput = tValidInputBase | test_utilities::create_valid_example_gradient_check_input();
    EXPECT_TRUE(input_validation::make_validated_input(tValidInput).hasValue());

    auto tGradientCheckInput = test_utilities::create_valid_example_gradient_check_input();
    tGradientCheckInput.random_direction_seed = 0;
    const auto tInvalidInput = tValidInputBase | tGradientCheckInput;
    EXPECT_TRUE(input_validation::make_validated_input(tInvalidInput).hasError());
}

}  // namespace plato::process_manager::extension::unittest
