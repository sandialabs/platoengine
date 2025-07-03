#include <gtest/gtest.h>

#include <filesystem>

#include "plato/criteria/library/test_utilities/ExampleInputBlocks.hpp"
#include "plato/geometry/extension/test_utilities/ExampleInputBlocks.hpp"
#include "plato/input_parser/InputBlockUtilities.hpp"
#include "plato/process_manager/extension/SensitivityCheck.hpp"
#include "plato/process_manager/extension/test_utilities/ExampleInputBlocks.hpp"
#include "plato/process_manager/library/ProcessManagerData.hpp"
#include "plato/test_utilities/FilesystemTestUtility.hpp"
#include "plato/test_utilities/TestContext.hpp"

namespace plato::process_manager::extension::unittest
{
TEST(SensitivityCheck, CreateSensitivityCheckRun)
{
    const auto tInputDeck = geometry::extension::test_utilities::create_valid_brick_shape_geometry_input() |
                            criteria::library::test_utilities::create_valid_example_objective_input() |
                            test_utilities::create_valid_example_sensitivity_check_input();

    const auto tValidatedInput = input_validation::make_validated_input(tInputDeck);
    ASSERT_TRUE(tValidatedInput.hasValue());
    const auto tAllProcessManagerInputs = tValidatedInput.value().get<components::ComponentType::kProcessManager>();
    EXPECT_EQ(tAllProcessManagerInputs.rawInput().size(), 1U);

    const auto tProblem = library::make_process_manager_data(tValidatedInput.value());
    const auto tSensitivityCheck = SensitivityCheck{tAllProcessManagerInputs.rawInput().back()};
    tSensitivityCheck.run(tProblem);

    const auto tSensitivityCheckFilePath = tAllProcessManagerInputs.rawInput()
                                               .front()
                                               .rawInput()
                                               .mInput.get<input_parser::sensitivity_check>()
                                               .output_file_name;

    plato::test_utilities::test_for_existence_and_remove({tSensitivityCheckFilePath.value().mToken},
                                                         TEST_CONTEXT("Sensitivity check file existence"));
}

TEST(SensitivityCheck, Registration)
{
    EXPECT_TRUE(library::is_process_manager_function_registered("sensitivity_check"));
}

}  // namespace plato::process_manager::extension::unittest
