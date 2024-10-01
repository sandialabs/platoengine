#include <gtest/gtest.h>

#include <fstream>

#include "plato/input_parser/InputBlockUtilities.hpp"
#include "plato/process_manager/extension/snopt/SNOPTOptimization.hpp"
#include "plato/process_manager/library/ProcessManagerData.hpp"
#include "plato/process_manager/library/ProcessManagerRegistration.hpp"
#include "plato/process_manager/library/ValidatedInput.hpp"
#include "plato/test_utilities/FilesystemTestUtility.hpp"
#include "plato/test_utilities/InputGeneration.hpp"
#include "plato/test_utilities/InputValidation.hpp"
#include "plato/test_utilities/TestContext.hpp"

namespace plato::process_manager::extension::snopt::unittest
{

namespace
{

constexpr std::string_view kSNOPTOptimizerFileName = "SNOPT_Optimization.txt";

}

TEST(SNOPTOptimization, ConstructAndRunSNOPTOptimization)
{
    const input_parser::ParsedInput tInputDeck = test_utilities::create_valid_brick_shape_geometry() |
                                                 test_utilities::create_valid_example_objective() |
                                                 test_utilities::create_valid_example_snopt_optimization();
    const auto tValidatedInput = library::make_validated_input(tInputDeck);
    const library::ProcessManagerData tProblem = library::make_process_manager_data(tValidatedInput);
    const auto tAllProcessManagerInputs = tValidatedInput.processManagers();
    ASSERT_EQ(tAllProcessManagerInputs.rawInput().size(), 1U);

    const auto tValidatedSNOPTSection =
        library::process_manager_input<input_parser::snopt_optimization>(tAllProcessManagerInputs.rawInput().front());
    const SNOPTOptimization tSNOPTOptimization{tValidatedSNOPTSection};
    tSNOPTOptimization.run(tProblem);

    test_utilities::test_for_existence_and_remove({std::string{kSNOPTOptimizerFileName}},
                                                  TEST_CONTEXT("Checking existence of SNOPT log file"));
}

TEST(SNOPTOptimizationDetail, MakeConstraints)
{
    {
        const input_parser::ParsedInput tInputDeck = test_utilities::create_valid_brick_shape_geometry() |
                                                     test_utilities::create_valid_example_objective() |
                                                     test_utilities::create_valid_example_snopt_optimization();
        const auto tValidatedInput = library::make_validated_input(tInputDeck);
        const library::ProcessManagerData tProblem = library::make_process_manager_data(tValidatedInput);

        const auto tConstraints = detail::make_constraints(tProblem);
        EXPECT_EQ(tConstraints.size(), 0U);
    }

    {
        const input_parser::ParsedInput tInputDeck = test_utilities::create_valid_brick_shape_geometry() |
                                                     test_utilities::create_valid_example_objective() |
                                                     test_utilities::create_valid_example_snopt_optimization() |
                                                     test_utilities::create_valid_example_constraint();
        const auto tValidatedInput = library::make_validated_input(tInputDeck);
        const library::ProcessManagerData tProblem = library::make_process_manager_data(tValidatedInput);

        const auto tConstraints = detail::make_constraints(tProblem);
        ASSERT_EQ(tConstraints.size(), 1U);
        const auto tSNOPTConstraint = tConstraints[0];
        EXPECT_EQ(tSNOPTConstraint.mLinearity, third_party_integration::snopt::Linearity::kLinear);
        EXPECT_EQ(tSNOPTConstraint.mTarget, 0.0);
        const linear_algebra::DynamicVector<double> tParameter({0.5, 0, 0, 1, 1, 1});
        const auto tResult = tSNOPTConstraint.mFunction.f(tParameter);

        constexpr auto tGoldNodalSumFromUnitBoxShiftedZeroPointFiveInX = double{4};
        EXPECT_EQ(tResult, tGoldNodalSumFromUnitBoxShiftedZeroPointFiveInX);
    }
}

TEST(SNOPTValidation, ValidateTimeLimit)
{
    constexpr bool tEmptyParameterGold = false;
    test_utilities::test_validation_function_using_valid_function_generator_vs_empty_struct<
        input_parser::snopt_optimization>([](const input_parser::snopt_optimization& aInput)
                                          { return detail::validate_time_limit_in_minutes(aInput); },
                                          []() { return test_utilities::create_valid_example_snopt_optimization(); },
                                          tEmptyParameterGold, TEST_CONTEXT("ValidateTimeLimit"));
}

}  // namespace plato::process_manager::extension::snopt::unittest
