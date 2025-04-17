#include <gtest/gtest.h>

#include "plato/criteria/library/ObjectiveInputBlock.hpp"
#include "plato/geometry/extension/BrickShapeGeometry.hpp"
#include "plato/input_parser/InputBlockUtilities.hpp"
#include "plato/process_manager/extension/GradientCheck.hpp"
#include "plato/process_manager/library/ProcessManagerValidation.hpp"
#include "plato/test_utilities/InputGeneration.hpp"

namespace plato::process_manager::library::unittest
{
TEST(ProcessManagerValidation, InValidParsedInputNoProcessManagers)
{
    const auto tInput = input_parser::NewParsedInput{};
    EXPECT_TRUE(detail::validate_at_least_one_process_manager(tInput).has_value());
}

TEST(ProcessManagerValidation, ValidateGradientCheck)
{
    const auto tValidInputBase = criteria::library::create_valid_example_objective_input() |
                                 geometry::extension::create_valid_brick_shape_geometry_input();

    const auto tValidInput = tValidInputBase | process_manager::extension::create_valid_example_gradient_check_input();
    EXPECT_TRUE(input_validation::make_validated_input(tValidInput).hasValue());

    auto tGradientCheckInput = process_manager::extension::create_valid_example_gradient_check_input();
    tGradientCheckInput.random_direction_seed = 0;
    const auto tInvalidInput =
        tValidInputBase | process_manager::extension::create_valid_example_gradient_check_input();
    EXPECT_TRUE(input_validation::make_validated_input(tValidInput).hasError());
}
}  // namespace plato::process_manager::library::unittest
