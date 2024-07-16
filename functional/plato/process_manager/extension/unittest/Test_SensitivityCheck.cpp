#include <gtest/gtest.h>

#include <filesystem>

#include "plato/input_parser/InputBlockUtilities.hpp"
#include "plato/process_manager/extension/SensitivityCheck.hpp"
#include "plato/process_manager/library/ProcessManagerData.hpp"
#include "plato/process_manager/library/ValidatedInput.hpp"
#include "plato/test_utilities/FilesystemTestUtility.hpp"
#include "plato/test_utilities/InputGeneration.hpp"
#include "plato/test_utilities/TestContext.hpp"

namespace plato::process_manager::extension::unittest
{
TEST(SensitivityCheck, CreateSensitivityCheckRun)
{
    const input_parser::ParsedInput tInputDeck = test_utilities::create_valid_brick_shape_geometry() |
                                                 test_utilities::create_valid_example_objective() |
                                                 test_utilities::create_valid_example_sensitivity_check();

    const auto tValidatedInput = library::make_validated_input(tInputDeck);
    const library::ValidatedProcessManagerInputVector tAllProcessManagerInputs = tValidatedInput.processManagers();
    EXPECT_EQ(tAllProcessManagerInputs.rawInput().size(), 1u);

    const library::ProcessManagerData tProblem = library::make_process_manager_data(tValidatedInput);
    const auto tSensitivityCheck = SensitivityCheck{
        library::process_manager_input<input_parser::sensitivity_check>(tAllProcessManagerInputs.rawInput().back())};
    tSensitivityCheck.run(tProblem);
    test_utilities::test_for_existence_and_remove(
        {tInputDeck.mSensitivityCheck.value().output_file_name.value().mToken},
        TEST_CONTEXT("Sensitivity check file existence"));
}

}  // namespace plato::process_manager::extension::unittest
