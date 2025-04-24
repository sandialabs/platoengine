#include <gtest/gtest.h>

#include "plato/criteria/library/test_utilities/ExampleInputBlocks.hpp"
#include "plato/geometry/extension/test_utilities/ExampleInputBlocks.hpp"
#include "plato/input_parser/InputBlockUtilities.hpp"
#include "plato/input_validation/ValidatedInput.hpp"
#include "plato/process_manager/extension/ConstraintCheck.hpp"
#include "plato/process_manager/extension/GradientCheck.hpp"
#include "plato/process_manager/extension/ROLOptimization.hpp"
#include "plato/process_manager/library/ProcessManagerFactory.hpp"
#include "plato/process_manager/library/ProcessManagerRegistration.hpp"

namespace plato::process_manager::library::unittest
{
TEST(ProcessManagerFactory, ValidProcessManagers)
{
    const auto tInput = geometry::extension::test_utilities::create_valid_brick_shape_geometry_input() |
                        criteria::library::test_utilities::create_valid_example_objective_input() |
                        process_manager::extension::create_valid_example_rol_optimization_input() |
                        process_manager::extension::create_valid_example_gradient_check_input() |
                        process_manager::extension::create_valid_example_constraint_check_input();

    const auto tData = input_validation::make_validated_input(tInput);
    ASSERT_TRUE(tData.hasValue());
    const auto tProcesses = make_process_managers(tData.value().get<input_parser::ComponentType::kProcessManager>());
    constexpr auto tExpectedNumberOfProcessManagers = 3U;
    EXPECT_EQ(tProcesses.size(), tExpectedNumberOfProcessManagers);
}

TEST(ProcessManagerFactory, NoProcessManagers)
{
    const auto tInput = geometry::extension::test_utilities::create_valid_brick_shape_geometry_input() |
                        criteria::library::test_utilities::create_valid_example_objective_input();

    const auto tData = input_validation::make_validated_input(tInput);
    EXPECT_TRUE(tData.hasError());
}

}  // namespace plato::process_manager::library::unittest
