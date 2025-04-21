#include <gtest/gtest.h>

#include <fstream>

#include "plato/criteria/library/ConstraintInputBlock.hpp"
#include "plato/criteria/library/ObjectiveInputBlock.hpp"
#include "plato/geometry/extension/BrickShapeGeometry.hpp"
#include "plato/input_parser/InputBlockUtilities.hpp"
#include "plato/process_manager/extension/snopt/SNOPTOptimization.hpp"
#include "plato/process_manager/library/ProcessManagerData.hpp"
#include "plato/process_manager/library/ProcessManagerRegistration.hpp"
#include "plato/test_utilities/FilesystemTestUtility.hpp"
#include "plato/test_utilities/InputGeneration.hpp"
#include "plato/test_utilities/InputValidation.hpp"
#include "plato/test_utilities/TestContext.hpp"

namespace plato::process_manager::extension::snopt::unittest
{

namespace
{
constexpr std::string_view kSNOPTOptimizerFileName = "SNOPT_Optimization.txt";

const auto kBaseInputDeck = geometry::extension::create_valid_brick_shape_geometry_input() |
                            criteria::library::create_valid_example_objective_input() |
                            create_valid_example_snopt_optimization_input();
}  // namespace

TEST(SNOPTOptimization, ConstructAndRunSNOPTOptimization)
{
    const auto tValidatedInput = input_validation::make_validated_input(kBaseInputDeck);
    const auto tProblem = library::make_process_manager_data(tValidatedInput.value());
    const auto tAllProcessManagerInputs = tValidatedInput.value().get<input_parser::ComponentType::kProcessManager>();
    ASSERT_EQ(tAllProcessManagerInputs.rawInput().size(), 1U);

    const auto tValidatedSNOPTSection = tAllProcessManagerInputs.rawInput().front();
    const auto tSNOPTOptimization = SNOPTOptimization{tValidatedSNOPTSection};
    tSNOPTOptimization.run(tProblem);

    test_utilities::test_for_existence_and_remove({std::string{kSNOPTOptimizerFileName}},
                                                  TEST_CONTEXT("Checking existence of SNOPT log file"));
}

TEST(SNOPTOptimizationDetail, MakeConstraints)
{
    {
        const auto tValidatedInput = input_validation::make_validated_input(kBaseInputDeck);
        const auto tProblem = library::make_process_manager_data(tValidatedInput.value());

        const auto tConstraints = detail::make_constraints(tProblem);
        EXPECT_TRUE(tConstraints.empty());
    }

    {
        const auto tInputDeck =
            input_parser::ParsedInput{kBaseInputDeck} | criteria::library::create_valid_example_constraint_input();
        const auto tValidatedInput = input_validation::make_validated_input(tInputDeck);
        ASSERT_TRUE(tValidatedInput.hasValue());
        const auto tProblem = library::make_process_manager_data(tValidatedInput.value());

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
        auto tConstraintInput = criteria::library::create_valid_example_constraint_input();
        tConstraintInput.constraint_type = aInputConstraintType;
        const auto tInputDeck = input_parser::ParsedInput{kBaseInputDeck} | tConstraintInput;
        const auto tValidatedInput = input_validation::make_validated_input(tInputDeck);
        const auto tProblem = library::make_process_manager_data(tValidatedInput.value());

        const auto tConstraints = detail::make_constraints(tProblem);
        ASSERT_EQ(tConstraints.size(), 1U) << aTestContext;
        EXPECT_EQ(tConstraints.front().mConstraintType, aSNOPTConstraintType) << aTestContext;
    };

    tCheckConstraintType(input_parser::ConstraintTypes::kEqualTo, tpis::ConstraintType::kEqualTo,
                         TEST_CONTEXT("Equal to"));
    tCheckConstraintType(input_parser::ConstraintTypes::kGreaterThan, tpis::ConstraintType::kGreaterThan,
                         TEST_CONTEXT("Greater than"));
    tCheckConstraintType(input_parser::ConstraintTypes::kLessThan, tpis::ConstraintType::kLesserThan,
                         TEST_CONTEXT("Lesser than"));
}

TEST(SNOPTValidation, ValidateTimeLimit)
{
    constexpr bool tEmptyParameterGold = false;
    test_utilities::test_validation_function_using_valid_function_generator_vs_empty_struct(
        [](const input_parser::snopt_optimization& aInput) { return detail::validate_time_limit_in_minutes(aInput); },
        create_valid_example_snopt_optimization_input(), tEmptyParameterGold, TEST_CONTEXT("ValidateTimeLimit"));
}

TEST(SNOPTOptimization, Registration)
{
    EXPECT_TRUE(library::is_process_manager_function_registered("snopt_optimization"));
    const auto tNames =
        core::registered_function_names<library::StageAndProcessManager, library::ValidatedProcessManagerInput>();
}

}  // namespace plato::process_manager::extension::snopt::unittest
