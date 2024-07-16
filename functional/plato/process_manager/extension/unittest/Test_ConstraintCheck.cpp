#include <gtest/gtest.h>

#include <ROL_Algorithm.hpp>

#include "plato/input_parser/InputBlockUtilities.hpp"
#include "plato/input_parser/InputBlocks.hpp"
#include "plato/process_manager/extension/ConstraintCheck.hpp"
#include "plato/process_manager/extension/LogspaceGenerator.hpp"
#include "plato/process_manager/library/ProcessManagerData.hpp"
#include "plato/process_manager/library/ProcessManagerRegistration.hpp"
#include "plato/process_manager/library/ValidatedInput.hpp"
#include "plato/test_utilities/FilesystemTestUtility.hpp"
#include "plato/test_utilities/InputGeneration.hpp"
#include "plato/test_utilities/TestContext.hpp"

namespace plato::process_manager::extension::unittest
{
void validate_and_run_constraint_check(const input_parser::ParsedInput& aInputDeck)
{
    const auto tValidatedInput = library::make_validated_input(aInputDeck);

    const library::ValidatedProcessManagerInputVector tAllProcessManagerInputs = tValidatedInput.processManagers();
    ASSERT_EQ(tAllProcessManagerInputs.rawInput().size(), 1u);

    const auto tConstraintCheck = ConstraintCheck{
        library::process_manager_input<input_parser::constraint_check>(tAllProcessManagerInputs.rawInput().back())};

    const library::ProcessManagerData tProblem = library::make_process_manager_data(tValidatedInput);
    tConstraintCheck.run(tProblem);
}

TEST(ConstraintCheck, ValidateAndRunChecksForLinearConstraint)
{
    namespace ptu = test_utilities;

    const input_parser::ParsedInput tInputDeck =
        ptu::create_valid_brick_shape_geometry() | ptu::create_valid_example_objective() |
        ptu::create_valid_example_constraint() | ptu::create_valid_example_constraint_check();

    validate_and_run_constraint_check(tInputDeck);

    ptu::test_for_existence_and_remove(
        {tInputDeck.mConstraintCheck.value().linearity_check_output_file_name.value().mToken},
        TEST_CONTEXT("Checking linearity check files"));
}

TEST(ConstraintCheck, ValidateAndRunChecksForNonlinearConstraint)
{
    namespace ptu = test_utilities;

    input_parser::ParsedInput tInputDeck = ptu::create_valid_brick_shape_geometry() |
                                           ptu::create_valid_example_objective() |
                                           ptu::create_valid_example_constraint_check();

    input_parser::constraint tNonlinearConstraint = ptu::create_valid_example_constraint();
    tNonlinearConstraint.is_linear = false;
    tInputDeck.mConstraints = {tNonlinearConstraint};

    validate_and_run_constraint_check(tInputDeck);

    ptu::test_for_existence_and_remove(
        {tInputDeck.mConstraintCheck.value().linearity_check_output_file_name.value().mToken,
         tInputDeck.mConstraintCheck.value().jacobian_check_output_file_name.value().mToken,
         tInputDeck.mConstraintCheck.value().jacobian_adjoint_consistency_output_file_name.value().mToken},
        TEST_CONTEXT("Checking constraint check files"));
}

}  // namespace plato::process_manager::extension::unittest