#include <gtest/gtest.h>

#include "plato/input_parser/InputBlockUtilities.hpp"
#include "plato/process_manager/library/ProcessManagerData.hpp"
#include "plato/process_manager/library/ProcessManagerFactory.hpp"
#include "plato/process_manager/library/ValidatedInput.hpp"
#include "plato/test_utilities/InputGeneration.hpp"

namespace plato::integration_tests::serial
{
namespace
{
input_parser::constraint create_mass_app_constraint_input()
{
    auto tConstraint = test_utilities::create_valid_example_constraint();
    tConstraint.name = "mass-app";
    tConstraint.constraint_value = 1;
    tConstraint.constraint_type = input_parser::ConstraintTypes::kLessThan;
    return tConstraint;
}

}  // namespace

TEST(Optimize, NodalSumObjective)
{
    const auto tInput = test_utilities::create_valid_example_objective() |
                        test_utilities::create_valid_brick_shape_geometry() |
                        test_utilities::create_valid_example_rol_optimization();
    const auto tValidatedInput = process_manager::library::make_validated_input(tInput);
    const auto tProcessManager = process_manager::library::make_process_managers(tValidatedInput.processManagers());
    ASSERT_EQ(tProcessManager.size(), 1u);
    const auto tROLOptimize = tProcessManager[0];
    const auto tProcessManagerData = process_manager::library::make_process_manager_data(tValidatedInput);
    tROLOptimize(tProcessManagerData);
}

TEST(Optimize, NodalSumObjectiveWithConstraint)
{
    const auto tInput = test_utilities::create_valid_example_objective() |
                        test_utilities::create_valid_brick_shape_geometry() |
                        test_utilities::create_valid_example_rol_optimization() | create_mass_app_constraint_input();

    const auto tValidatedInput = process_manager::library::make_validated_input(tInput);
    const auto tProcessManager = process_manager::library::make_process_managers(tValidatedInput.processManagers());
    ASSERT_EQ(tProcessManager.size(), 1u);
    const auto tROLOptimize = tProcessManager[0];
    const auto tProcessManagerData = process_manager::library::make_process_manager_data(tValidatedInput);
    tROLOptimize(tProcessManagerData);
}

}  // namespace plato::integration_tests::serial
