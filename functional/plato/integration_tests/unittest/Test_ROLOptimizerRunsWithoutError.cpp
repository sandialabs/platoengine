#include <gtest/gtest.h>

#include "plato/criteria/library/test_utilities/ExampleInputBlocks.hpp"
#include "plato/geometry/extension/test_utilities/ExampleInputBlocks.hpp"
#include "plato/input_parser/InputBlockUtilities.hpp"
#include "plato/input_validation/ValidatedInput.hpp"
#include "plato/process_manager/extension/ROLOptimization.hpp"
#include "plato/process_manager/library/ProcessManagerData.hpp"
#include "plato/process_manager/library/ProcessManagerFactory.hpp"
#include "plato/test_utilities/InputGeneration.hpp"
#include "plato/test_utilities/TestContext.hpp"

namespace plato::integration_tests::serial
{
namespace
{
constexpr auto kROLOutputFile = std::string_view{"ROL_Optimizer.txt"};

auto create_mass_app_constraint_input() -> input_parser::constraint
{
    auto tConstraint = criteria::library::test_utilities::create_valid_example_constraint_input();
    tConstraint.name = "mass-app";
    tConstraint.constraint_value = 1;
    tConstraint.constraint_type = input_parser::ConstraintTypes::kLessThan;
    return tConstraint;
}

void check_optimization_runs_without_error(const input_parser::ParsedInput& aInput,
                                           const test_utilities::TestContext& aTestContext)
{
    const auto tValidatedInput = input_validation::make_validated_input(aInput).value();
    const auto tProcessManager = process_manager::library::make_process_managers(
        tValidatedInput.get<input_parser::ComponentType::kProcessManager>());
    ASSERT_EQ(tProcessManager.size(), 1U) << aTestContext;
    const auto& tROLOptimize = tProcessManager.front();
    const auto tProcessManagerData = process_manager::library::make_process_manager_data(tValidatedInput);
    tROLOptimize(tProcessManagerData);

    EXPECT_TRUE(std::filesystem::remove(kROLOutputFile)) << aTestContext;
    EXPECT_TRUE(std::filesystem::remove(
        aInput.get<input_parser::rol_optimization>().front().export_settings_file_name->mToken));
}

}  // namespace

TEST(ROLOptimizerRunsWithoutError, NodalSumObjective)
{
    const auto tInput = criteria::library::test_utilities::create_valid_example_objective_input() |
                        geometry::extension::test_utilities::create_valid_brick_shape_geometry_input() |
                        process_manager::extension::create_valid_example_rol_optimization_input();
    check_optimization_runs_without_error(tInput, TEST_CONTEXT("Nodal sum objective"));
}

TEST(ROLOptimizerRunsWithoutError, NodalSumObjectiveWithConstraint)
{
    const auto tInput = criteria::library::test_utilities::create_valid_example_objective_input() |
                        geometry::extension::test_utilities::create_valid_brick_shape_geometry_input() |
                        process_manager::extension::create_valid_example_rol_optimization_input() |
                        create_mass_app_constraint_input();
    check_optimization_runs_without_error(tInput, TEST_CONTEXT("Nodal sum with constraint"));
}

}  // namespace plato::integration_tests::serial
