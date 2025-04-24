#include <gtest/gtest.h>

#include "plato/criteria/library/test_utilities/ExampleInputBlocks.hpp"
#include "plato/geometry/extension/test_utilities/ExampleInputBlocks.hpp"
#include "plato/input_parser/InputBlockUtilities.hpp"
#include "plato/process_manager/extension/test_utilities/ExampleInputBlocks.hpp"

namespace plato::integration_tests::unittest
{
namespace
{
const auto kValidInputBase = geometry::extension::test_utilities::create_valid_brick_shape_geometry_input() |
                             criteria::library::test_utilities::create_valid_example_objective_input() |
                             process_manager::extension::test_utilities::create_valid_example_rol_optimization_input();
}

TEST(ConstraintValidation, ValidInput)
{
    const auto tValidInput =
        kValidInputBase | criteria::library::test_utilities::create_valid_example_constraint_input();
    const auto tValidatedInput = input_validation::make_validated_input(tValidInput);
    EXPECT_TRUE(tValidatedInput.hasValue()) << tValidatedInput.error();
}

TEST(ConstraintValidation, NoErrorMessagesTwoValidConstraints)
{
    const auto tValidInput = kValidInputBase |
                             criteria::library::test_utilities::create_valid_example_constraint_input() |
                             criteria::library::test_utilities::create_valid_example_constraint_input();
    EXPECT_TRUE(input_validation::make_validated_input(tValidInput).hasValue());
}

TEST(ConstraintValidation, ErrorMessagesInvalidConstraint)
{
    auto tConstraint = criteria::library::test_utilities::create_valid_example_constraint_input();
    tConstraint.app = boost::none;
    tConstraint.criterion = boost::none;
    const auto tInvalidInput = kValidInputBase | tConstraint;
    EXPECT_TRUE(input_validation::make_validated_input(tInvalidInput).hasError());
}

TEST(ConstraintValidation, ErrorMessagesTwoInvalidInput)
{
    auto tConstraint = input_parser::constraint{};
    tConstraint.name = "bad-one";
    auto tConstraintTwo = tConstraint;
    tConstraintTwo.name = "bad-two";
    const auto tInvalidInput = kValidInputBase | tConstraint | tConstraintTwo;

    EXPECT_TRUE(input_validation::make_validated_input(tInvalidInput).hasError());
}

}  // namespace plato::integration_tests::unittest
