#include <gtest/gtest.h>

#include <ROL_Algorithm.hpp>

#include "plato/criteria/library/ConstraintInputBlock.hpp"
#include "plato/criteria/library/ObjectiveInputBlock.hpp"
#include "plato/geometry/extension/BrickShapeGeometry.hpp"
#include "plato/input_parser/InputBlockUtilities.hpp"
#include "plato/input_parser/ParsedInput.hpp"
#include "plato/process_manager/extension/ConstraintCheck.hpp"
#include "plato/process_manager/extension/LogspaceGenerator.hpp"
#include "plato/process_manager/library/ProcessManagerData.hpp"
#include "plato/process_manager/library/ProcessManagerRegistration.hpp"
#include "plato/test_utilities/FilesystemTestUtility.hpp"
#include "plato/test_utilities/TestContext.hpp"

namespace plato::process_manager::extension::unittest
{
namespace
{
const auto kBaseInputDeck = geometry::extension::create_valid_brick_shape_geometry_input() |
                            criteria::library::create_valid_example_objective_input() |
                            criteria::library::create_valid_example_constraint_input() |
                            create_valid_example_constraint_check_input();
}

void validate_and_run_constraint_check(const input_parser::NewParsedInput& aInputDeck)
{
    const auto tValidatedInput = input_validation::make_validated_input(aInputDeck);
    ASSERT_TRUE(tValidatedInput.hasValue());

    const auto tAllProcessManagerInputs = tValidatedInput.value().get<input_parser::ComponentType::kProcessManager>();
    ASSERT_EQ(tAllProcessManagerInputs.rawInput().size(), 1U);

    const auto tConstraintCheck = ConstraintCheck{tAllProcessManagerInputs.rawInput().front()};

    const auto tProblem = library::make_process_manager_data(tValidatedInput.value());
    tConstraintCheck.run(tProblem);
}

TEST(ConstraintCheck, ValidateAndRunChecksForLinearConstraint)
{
    namespace ptu = test_utilities;

    validate_and_run_constraint_check(kBaseInputDeck);

    const auto tLinearityCheckFilePath =
        kBaseInputDeck.get<input_parser::new_constraint_check>().front().linearity_check_output_file_name;
    ASSERT_TRUE(tLinearityCheckFilePath.has_value());
    ptu::test_for_existence_and_remove({tLinearityCheckFilePath.value().mToken},
                                       TEST_CONTEXT("Checking linearity check files"));
}

TEST(ConstraintCheck, ValidateAndRunChecksForNonlinearConstraint)
{
    namespace ptu = test_utilities;

    const auto tCheckFunction =
        [](const input_parser::constraint& aConstraint, const test_utilities::TestContext& aTestContext)
    {
        const auto tInputDeck = kBaseInputDeck | aConstraint;

        validate_and_run_constraint_check(tInputDeck);

        const auto tConstraintCheckInput = kBaseInputDeck.get<input_parser::new_constraint_check>().front();

        ptu::test_for_existence_and_remove(
            {tConstraintCheckInput.linearity_check_output_file_name.value().mToken,
             tConstraintCheckInput.jacobian_check_output_file_name.value().mToken,
             tConstraintCheckInput.jacobian_adjoint_consistency_output_file_name.value().mToken},
            EXTEND_CONTEXT("Checking constraint check files", aTestContext));
    };

    {
        auto tNonlinearConstraint = criteria::library::create_valid_example_constraint_input();
        tNonlinearConstraint.is_linear = false;
        tCheckFunction(tNonlinearConstraint, TEST_CONTEXT("Non-linear equality constraint"));
    }
    {
        auto tNonlinearConstraint = criteria::library::create_valid_example_constraint_input();
        tNonlinearConstraint.is_linear = false;
        tNonlinearConstraint.constraint_type = input_parser::ConstraintTypes::kLessThan;
        tCheckFunction(tNonlinearConstraint, TEST_CONTEXT("Non-linear inequality constraint"));
    }
}

TEST(ConstraintCheck, Registration)
{
    EXPECT_TRUE(library::is_new_process_manager_function_registered("new_constraint_check"));
}

}  // namespace plato::process_manager::extension::unittest
