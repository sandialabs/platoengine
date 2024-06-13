#include <gtest/gtest.h>

#include "plato/input_parser/InputBlocks.hpp"
#include "plato/process_manager/extension/CommonInputValidation.hpp"

namespace plato::process_manager::extension::unittest
{
TEST(ValidateCommonInput, ValidateNumberOfSteps)
{
    input_parser::gradient_check tGradientCheck;
    EXPECT_TRUE(detail::validate_number_of_steps(tGradientCheck).has_value());

    input_parser::constraint_check tConstraintCheck;
    EXPECT_TRUE(detail::validate_number_of_steps(tConstraintCheck).has_value());

    tConstraintCheck.number_of_steps = 1u;  // in bounds
    EXPECT_FALSE(detail::validate_number_of_steps(tConstraintCheck).has_value());

    tGradientCheck.number_of_steps = 0;  // out of bounds
    EXPECT_TRUE(detail::validate_number_of_steps(tGradientCheck).has_value());
}

TEST(ValidateCommonInput, ValidateInitialDirectionMagnitude)
{
    input_parser::gradient_check tGradientCheck;
    EXPECT_TRUE(detail::validate_initial_direction_magnitude(tGradientCheck).has_value());

    input_parser::constraint_check tConstraintCheck;
    EXPECT_TRUE(detail::validate_initial_direction_magnitude(tConstraintCheck).has_value());

    tGradientCheck.initial_direction_magnitude = 0;  // out of bounds
    EXPECT_TRUE(detail::validate_initial_direction_magnitude(tGradientCheck).has_value());

    tConstraintCheck.initial_direction_magnitude = 10;  // in bounds
    EXPECT_FALSE(detail::validate_initial_direction_magnitude(tConstraintCheck).has_value());

    tConstraintCheck.initial_direction_magnitude = -1.0;  // out of bounds
    EXPECT_TRUE(detail::validate_initial_direction_magnitude(tConstraintCheck).has_value());
}

TEST(ValidateCommonInput, ValidateStepSizeReductionFactor)
{
    input_parser::gradient_check tGradientCheck;
    EXPECT_TRUE(detail::validate_step_size_reduction_factor(tGradientCheck).has_value());

    input_parser::constraint_check tConstraintCheck;
    EXPECT_TRUE(detail::validate_initial_direction_magnitude(tConstraintCheck).has_value());

    tConstraintCheck.step_size_reduction_factor = 0.5;  // in bounds
    EXPECT_FALSE(detail::validate_step_size_reduction_factor(tConstraintCheck).has_value());

    tGradientCheck.step_size_reduction_factor = 10;  // out of bounds
    EXPECT_TRUE(detail::validate_step_size_reduction_factor(tGradientCheck).has_value());

    tConstraintCheck.step_size_reduction_factor = -0.1;  // out of bounds
    EXPECT_TRUE(detail::validate_step_size_reduction_factor(tConstraintCheck).has_value());
}

TEST(ValidateCommonInput, ValidateRandomDirectionSeed)
{
    input_parser::gradient_check tGradientCheck;
    EXPECT_TRUE(detail::validate_step_size_reduction_factor(tGradientCheck).has_value());

    input_parser::constraint_check tConstraintCheck;
    EXPECT_TRUE(detail::validate_initial_direction_magnitude(tConstraintCheck).has_value());

    tConstraintCheck.random_direction_seed = 1;  // in bounds
    EXPECT_FALSE(detail::validate_random_direction_seed(tConstraintCheck).has_value());

    tGradientCheck.random_direction_seed = 0;  // out of bounds
    EXPECT_TRUE(detail::validate_random_direction_seed(tGradientCheck).has_value());

    tConstraintCheck.random_direction_seed = 1234567;  // in bounds
    EXPECT_FALSE(detail::validate_random_direction_seed(tConstraintCheck).has_value());
}

}  // namespace plato::process_manager::extension::unittest
