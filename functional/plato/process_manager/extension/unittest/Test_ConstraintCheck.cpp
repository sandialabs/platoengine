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
namespace
{
const input_parser::ParsedInput kBaseInputDeck =
    test_utilities::create_valid_brick_shape_geometry() | test_utilities::create_valid_example_objective() |
    test_utilities::create_valid_example_constraint() | test_utilities::create_valid_example_constraint_check();
}

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

    validate_and_run_constraint_check(kBaseInputDeck);

    ptu::test_for_existence_and_remove(
        {kBaseInputDeck.mConstraintCheck.value().linearity_check_output_file_name.value().mToken},
        TEST_CONTEXT("Checking linearity check files"));
}

TEST(ConstraintCheck, ValidateAndRunChecksForNonlinearConstraint)
{
    namespace ptu = test_utilities;

    const auto tCheckFunction =
        [](const input_parser::constraint& aConstraint, const test_utilities::TestContext& aTestContext)
    {
        const auto tInputDeck = input_parser::ParsedInput{kBaseInputDeck} | aConstraint;

        validate_and_run_constraint_check(tInputDeck);

        ptu::test_for_existence_and_remove(
            {tInputDeck.mConstraintCheck.value().linearity_check_output_file_name.value().mToken,
             tInputDeck.mConstraintCheck.value().jacobian_check_output_file_name.value().mToken,
             tInputDeck.mConstraintCheck.value().jacobian_adjoint_consistency_output_file_name.value().mToken},
            EXTEND_CONTEXT("Checking constraint check files", aTestContext));
    };

    {
        input_parser::constraint tNonlinearConstraint = ptu::create_valid_example_constraint();
        tNonlinearConstraint.is_linear = false;
        tCheckFunction(tNonlinearConstraint, TEST_CONTEXT("Non-linear equality constraint"));
    }
    {
        input_parser::constraint tNonlinearConstraint = ptu::create_valid_example_constraint();
        tNonlinearConstraint.is_linear = false;
        tNonlinearConstraint.constraint_type = input_parser::ConstraintTypes::kLessThan;
        tCheckFunction(tNonlinearConstraint, TEST_CONTEXT("Non-linear inequality constraint"));
    }
}

}  // namespace plato::process_manager::extension::unittest
