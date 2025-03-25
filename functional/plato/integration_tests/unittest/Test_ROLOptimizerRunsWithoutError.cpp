#include <gtest/gtest.h>

#include "plato/input_parser/InputBlockUtilities.hpp"
#include "plato/process_manager/library/ProcessManagerData.hpp"
#include "plato/process_manager/library/ProcessManagerFactory.hpp"
#include "plato/process_manager/library/ValidatedInput.hpp"
#include "plato/test_utilities/InputGeneration.hpp"
#include "plato/test_utilities/TestContext.hpp"

namespace plato::integration_tests::serial
{
namespace
{
constexpr auto kROLOutputFile = std::string_view{"ROL_Optimizer.txt"};
auto create_mass_app_constraint_input() -> input_parser::constraint
{
    auto tConstraint = test_utilities::create_valid_example_constraint();
    tConstraint.name = "mass-app";
    tConstraint.constraint_value = 1;
    tConstraint.constraint_type = input_parser::ConstraintTypes::kLessThan;
    return tConstraint;
}

void check_optimization_runs_without_error(const input_parser::ParsedInput& aInput,
                                           const test_utilities::TestContext& aTestContext)
{
    const auto tValidatedInput = process_manager::library::make_validated_input(aInput);
    const auto tProcessManager = process_manager::library::make_process_managers(tValidatedInput.processManagers());
    ASSERT_EQ(tProcessManager.size(), 1u) << aTestContext;
    const auto& tROLOptimize = tProcessManager.front();
    const auto tProcessManagerData = process_manager::library::make_process_manager_data(tValidatedInput);
    tROLOptimize(tProcessManagerData);

    EXPECT_TRUE(std::filesystem::remove(kROLOutputFile)) << aTestContext;
    EXPECT_TRUE(std::filesystem::remove(aInput.mROLOptimization->export_settings_file_name->mToken));
}

}  // namespace

TEST(ROLOptimizerRunsWithoutError, NodalSumObjective)
{
    const auto tInput = test_utilities::create_valid_example_objective() |
                        test_utilities::create_valid_brick_shape_geometry() |
                        test_utilities::create_valid_example_rol_optimization();
    check_optimization_runs_without_error(tInput, TEST_CONTEXT("Nodal sum objective"));
}

TEST(ROLOptimizerRunsWithoutError, NodalSumObjectiveWithConstraint)
{
    const auto tInput = test_utilities::create_valid_example_objective() |
                        test_utilities::create_valid_brick_shape_geometry() |
                        test_utilities::create_valid_example_rol_optimization() | create_mass_app_constraint_input();
    check_optimization_runs_without_error(tInput, TEST_CONTEXT("Nodal sum with constraint"));
}

}  // namespace plato::integration_tests::serial
