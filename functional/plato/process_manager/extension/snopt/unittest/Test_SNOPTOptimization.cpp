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

const auto kBaseInputDeck = test_utilities::create_valid_brick_shape_geometry() |
                            test_utilities::create_valid_example_objective() |
                            test_utilities::create_valid_example_snopt_optimization();
}  // namespace

TEST(SNOPTOptimization, ConstructAndRunSNOPTOptimization)
{
    const auto tValidatedInput = library::make_validated_input(kBaseInputDeck);
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
        const auto tValidatedInput = library::make_validated_input(kBaseInputDeck);
        const library::ProcessManagerData tProblem = library::make_process_manager_data(tValidatedInput);

        const auto tConstraints = detail::make_constraints(tProblem);
        EXPECT_EQ(tConstraints.size(), 0U);
    }

    {
        const input_parser::ParsedInput tInputDeck =
            input_parser::ParsedInput{kBaseInputDeck} | test_utilities::create_valid_example_constraint();
        const auto tValidatedInput = library::make_validated_input(tInputDeck);
        const library::ProcessManagerData tProblem = library::make_process_manager_data(tValidatedInput);

        const auto tConstraints = detail::make_constraints(tProblem);
        ASSERT_EQ(tConstraints.size(), 1U);
        const auto& tSNOPTConstraint = tConstraints[0];
        EXPECT_EQ(tSNOPTConstraint.mLinearity, third_party_integration::snopt::Linearity::kLinear);
        EXPECT_EQ(tSNOPTConstraint.mConstraintType, third_party_integration::snopt::ConstraintType::kEqualTo);
        constexpr auto tExpectedTargetsSize = 1U;
        ASSERT_EQ(tSNOPTConstraint.mTargets.size(), 1U);
        EXPECT_EQ(tSNOPTConstraint.mTargets.front(), 0.0);

        const linear_algebra::DynamicVector<double> tParameter({0.5, 0, 0, 1, 1, 1});
        const auto tResult = tSNOPTConstraint.mFunction.template evaluate<core::evaluation::kFunction>(tParameter);
        constexpr auto tGoldNodalSumFromUnitBoxShiftedZeroPointFiveInX = double{4};
        ASSERT_EQ(tResult.size(), tExpectedTargetsSize);
        EXPECT_EQ(tResult[0], tGoldNodalSumFromUnitBoxShiftedZeroPointFiveInX);
    }
}

TEST(SNOPTOptimizationDetail, ConstraintType)
{
    namespace tpis = third_party_integration::snopt;
    const auto tCheckConstraintType = [](const input_parser::ConstraintTypes aInputConstraintType,
                                         const tpis::ConstraintType aSNOPTConstraintType,
                                         const test_utilities::TestContext& aTestContext)
    {
        auto tConstraintInput = test_utilities::create_valid_example_constraint();
        tConstraintInput.constraint_type = aInputConstraintType;
        const input_parser::ParsedInput tInputDeck = input_parser::ParsedInput{kBaseInputDeck} | tConstraintInput;
        const auto tValidatedInput = library::make_validated_input(tInputDeck);
        const library::ProcessManagerData tProblem = library::make_process_manager_data(tValidatedInput);

        const auto tConstraints = detail::make_constraints(tProblem);
        ASSERT_EQ(tConstraints.size(), 1U) << aTestContext;
        EXPECT_EQ(tConstraints.front().mConstraintType, aSNOPTConstraintType) << aTestContext;
    };

    tCheckConstraintType(input_parser::ConstraintTypes::kEquality, tpis::ConstraintType::kEqualTo,
                         TEST_CONTEXT("Equal to"));
    tCheckConstraintType(input_parser::ConstraintTypes::kGreaterThan, tpis::ConstraintType::kGreaterThan,
                         TEST_CONTEXT("Greater than"));
    tCheckConstraintType(input_parser::ConstraintTypes::kLessThan, tpis::ConstraintType::kLesserThan,
                         TEST_CONTEXT("Lesser than"));
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
