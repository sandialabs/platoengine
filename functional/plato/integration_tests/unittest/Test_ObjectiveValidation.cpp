#include <gtest/gtest.h>

#include "plato/criteria/library/test_utilities/ExampleInputBlocks.hpp"
#include "plato/geometry/extension/test_utilities/ExampleInputBlocks.hpp"
#include "plato/input_parser/InputBlockUtilities.hpp"
#include "plato/input_validation/ValidatedInput.hpp"
#include "plato/process_manager/extension/test_utilities/ExampleInputBlocks.hpp"

namespace plato::integration_tests::unittest
{
TEST(ObjectiveValidation, ErrorMessagesInvalidObjective)
{
    const auto tInputBase = geometry::extension::test_utilities::create_valid_brick_shape_geometry_input() |
                            process_manager::extension::test_utilities::create_valid_example_rol_optimization_input();
    const auto tValidInput = tInputBase | criteria::library::test_utilities::create_valid_example_objective_input();
    EXPECT_TRUE(input_validation::make_validated_input(tValidInput).hasValue());

    // No criterion
    {
        auto tObjective = criteria::library::test_utilities::create_valid_example_objective_input();
        tObjective.criterion = boost::none;
        const auto tInvalidInput = tInputBase | tObjective;
        EXPECT_TRUE(input_validation::make_validated_input(tInvalidInput).hasError());
    }
    // No active objective
    {
        auto tObjective = criteria::library::test_utilities::create_valid_example_objective_input();
        tObjective.active = false;
        const auto tInvalidInput = tInputBase | tObjective;
        EXPECT_TRUE(input_validation::make_validated_input(tInvalidInput).hasError());
    }
}

}  // namespace plato::integration_tests::unittest
