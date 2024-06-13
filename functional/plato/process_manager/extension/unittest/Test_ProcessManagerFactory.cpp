#include <gtest/gtest.h>

#include "plato/input_parser/InputBlockUtilities.hpp"
#include "plato/process_manager/library/ProcessManagerFactory.hpp"
#include "plato/process_manager/library/ProcessManagerRegistration.hpp"
#include "plato/process_manager/library/ValidatedInput.hpp"
#include "plato/test_utilities/InputGeneration.hpp"

namespace plato::process_manager::library::unittest
{

TEST(ProcessManagerFactory, ValidOptimization)
{
    const input_parser::ParsedInput tInput = test_utilities::create_valid_example_input();
    const ValidatedInput tData = make_validated_input(tInput);
    const auto tProcesses = make_process_managers(tData.processManagers());
    EXPECT_EQ(tProcesses.size(), 1u);
}

TEST(ProcessManagerFactory, ValidOptimizationAndGradientCheck)
{
    const input_parser::ParsedInput tInput =
        test_utilities::create_valid_brick_shape_geometry() | test_utilities::create_valid_example_objective() |
        test_utilities::create_valid_example_gradient_check() | test_utilities::create_valid_example_rol_optimization();
    const ValidatedInput tData = make_validated_input(tInput);
    const auto tProcesses = make_process_managers(tData.processManagers());
    EXPECT_EQ(tProcesses.size(), 2u);
}

TEST(ProcessManagerFactory, ValidOptimizationAndConstraintCheck)
{
    const input_parser::ParsedInput tInput = test_utilities::create_valid_brick_shape_geometry() |
                                             test_utilities::create_valid_example_objective() |
                                             test_utilities::create_valid_example_constraint_check();

    const ValidatedInput tData = make_validated_input(tInput);
    const auto tProcesses = make_process_managers(tData.processManagers());
    EXPECT_EQ(tProcesses.size(), 1u);
}

}  // namespace plato::process_manager::library::unittest