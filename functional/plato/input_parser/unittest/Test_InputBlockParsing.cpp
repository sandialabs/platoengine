#include <gtest/gtest.h>

#include "plato/input_parser/InputBlocks.hpp"
#include "plato/input_parser/unittest/Test_Helpers.hpp"

namespace plato::input_parser::unittest
{
TEST(ParseObjectiveInputBlock, AllValidInputs)
{
    const std::string tInput =
        R"(
          begin objective mp_objective
            active true
            app platoengine
            criterion volume
            number_of_processors 10
            input_files test.txt, test2.xml
            objective_type minimize
            aggregation_weight 10.0
          end
       )";

    // Parse
    const auto [tParseResult, tIter, tData] = parse_string(tInput);

    // Tests
    EXPECT_TRUE(tParseResult);
    EXPECT_EQ(tIter, tInput.cend());

    ASSERT_EQ(tData.mObjectives.size(), 1u);
    const auto& tObjective = tData.mObjectives.front();
    test_existence_and_equality(tObjective.name, "mp_objective");
    test_existence_and_equality(tObjective.app, std::string{"platoengine"});
    test_existence_and_equality(tObjective.criterion, std::string{"volume"});
    test_existence_and_equality(tObjective.number_of_processors, 10u);
    test_existence_and_equality(tObjective.active, true);
    test_existence_and_equality(tObjective.input_files, std::vector<std::string>{"test.txt", "test2.xml"});
    test_existence_and_equality(tObjective.objective_type, ObjectiveTypes::kMinimize);
    test_existence_and_equality(tObjective.aggregation_weight, 10.0);
}

TEST(ParseObjectiveInputBlock, NotAllInputs)
{
    const std::string tInput =
        R"(
          begin objective mp_objective
            app fancy-app
            criterion quantum-gravitational-turbo-dynamics
            number_of_processors 10
            aggregation_weight 10.0
          end
       )";

    // Parse
    const auto [tParseResult, tIter, tData] = parse_string(tInput);

    ASSERT_EQ(tData.mObjectives.size(), 1u);
    const auto& tObjective = tData.mObjectives.front();
    test_existence_and_equality(tObjective.name, "mp_objective");
    test_existence_and_equality(tObjective.app, std::string{"fancy-app"});
    test_existence_and_equality(tObjective.criterion, std::string{"quantum-gravitational-turbo-dynamics"});
    test_existence_and_equality(tObjective.number_of_processors, 10u);
    EXPECT_FALSE(tObjective.active);
    EXPECT_FALSE(tObjective.input_files);
    EXPECT_FALSE(tObjective.objective_type);
    test_existence_and_equality(tObjective.aggregation_weight, 10.0);
}

TEST(ParseConstraintInputBlock, AllValidInputs)
{
    const std::string tInput =
        R"(
          begin constraint mp_constraint
            active true
            app volume_fraction
            number_of_processors 10
            input_files test.txt
            equal_to 1.0
            is_linear false
          end
       )";

    // Parse
    const auto [tParseResult, tIter, tData] = parse_string(tInput);

    // Tests
    EXPECT_TRUE(tParseResult);
    EXPECT_EQ(tIter, tInput.cend());

    ASSERT_EQ(tData.mConstraints.size(), 1u);
    const auto& tConstraint = tData.mConstraints.front();
    test_existence_and_equality(tConstraint.name, "mp_constraint");
    test_existence_and_equality(tConstraint.active, true);
    test_existence_and_equality(tConstraint.app, std::string{"volume_fraction"});
    test_existence_and_equality(tConstraint.number_of_processors, 10u);
    test_existence_and_equality(tConstraint.input_files, std::vector<std::string>{"test.txt"});
    test_existence_and_equality(tConstraint.equal_to, 1.0);
    test_existence_and_equality(tConstraint.is_linear, false);
}

TEST(ParseConstraintInputBlock, MultipleBlocks)
{
    const std::string tInput =
        R"(
          begin constraint mp_constraint_1
            active true
            app volume
          end
          begin constraint mp_constraint_2
            number_of_processors 10
            equal_to -10.0
          end

       )";

    // Parse
    const auto [tParseResult, tIter, tData] = parse_string(tInput);

    // Tests
    EXPECT_TRUE(tParseResult);
    EXPECT_EQ(tIter, tInput.cend());

    ASSERT_EQ(tData.mConstraints.size(), 2u);
    const auto& tConstraint1 = tData.mConstraints.front();
    test_existence_and_equality(tConstraint1.name, "mp_constraint_1");
    test_existence_and_equality(tConstraint1.active, true);
    const auto& tConstraint2 = tData.mConstraints.back();
    test_existence_and_equality(tConstraint2.name, "mp_constraint_2");
    test_existence_and_equality(tConstraint2.number_of_processors, 10u);
    test_existence_and_equality(tConstraint2.equal_to, -10.0);
}

TEST(ParseRolOptimizationInputBlock, AllValidInputs)
{
    const std::string tInput =
        R"(
          begin rol_optimization
            input_file_name its-a_file.txt
            max_iterations 100
            step_tolerance 
            
            10
            gradient_tolerance 100.0
          end
       )";

    // Parse
    const auto [tParseResult, tIter, tData] = parse_string(tInput);

    // Tests
    EXPECT_TRUE(tParseResult);
    EXPECT_EQ(tIter, tInput.end());
    ASSERT_TRUE(tData.mROLOptimization);
    test_existence_and_equality(tData.mROLOptimization->input_file_name, std::string{"its-a_file.txt"});
    test_existence_and_equality(tData.mROLOptimization->max_iterations, 100u);
    test_existence_and_equality(tData.mROLOptimization->step_tolerance, 10.0);
    test_existence_and_equality(tData.mROLOptimization->gradient_tolerance, 100.0);
}

TEST(ParseGradientCheckInputBlock, AllValidInputs)
{
    const std::string tInput =
        R"(
          begin gradient_check
            output_file_name its-a_file.txt
            number_of_steps 10
            step_size_reduction_factor 0.5
            random_direction_seed 1
            initial_direction_magnitude 0.5
          end
       )";

    // Parse
    const auto [tParseResult, tIter, tData] = parse_string(tInput);

    // Tests
    EXPECT_TRUE(tParseResult);
    EXPECT_EQ(tIter, tInput.end());
    ASSERT_TRUE(tData.mGradientCheck);
    test_existence_and_equality(tData.mGradientCheck->output_file_name, std::string{"its-a_file.txt"});
    test_existence_and_equality(tData.mGradientCheck->number_of_steps, 10u);
    test_existence_and_equality(tData.mGradientCheck->step_size_reduction_factor, 0.5);
    test_existence_and_equality(tData.mGradientCheck->random_direction_seed, 1u);
    test_existence_and_equality(tData.mGradientCheck->initial_direction_magnitude, 0.5);
}

TEST(ParseConstraintCheckInputBlock, AllValidInputs)
{
    const std::string tInput =
        R"(
          begin constraint_check
            linearity_check_output_file_name ROL_constraint_linearity_check_output.txt
            jacobian_check_output_file_name ROL_constraint_jacobian_check_output.txt
            jacobian_adjoint_consistency_output_file_name ROL_constraint_jacobian_adjoint_consistency_check_output.txt
            step_size_reduction_factor 0.1
            number_of_steps 10
            initial_direction_magnitude 1
            random_direction_seed 123
          end
       )";

    // Parse
    const auto [tParseResult, tIter, tData] = parse_string(tInput);

    // Tests
    EXPECT_TRUE(tParseResult);
    EXPECT_EQ(tIter, tInput.end());
    ASSERT_TRUE(tData.mConstraintCheck);
    test_existence_and_equality(tData.mConstraintCheck->linearity_check_output_file_name,
                                std::string{"ROL_constraint_linearity_check_output.txt"});
    test_existence_and_equality(tData.mConstraintCheck->jacobian_check_output_file_name,
                                std::string{"ROL_constraint_jacobian_check_output.txt"});
    test_existence_and_equality(tData.mConstraintCheck->jacobian_adjoint_consistency_output_file_name,
                                std::string{"ROL_constraint_jacobian_adjoint_consistency_check_output.txt"});
    test_existence_and_equality(tData.mConstraintCheck->number_of_steps, 10u);
    test_existence_and_equality(tData.mConstraintCheck->initial_direction_magnitude, 1.0);
    test_existence_and_equality(tData.mConstraintCheck->step_size_reduction_factor, 0.1);
    test_existence_and_equality(tData.mConstraintCheck->random_direction_seed, 123u);
}

TEST(ParseIdentityFilterInputBlock, NoInputs)
{
    const std::string tInput =
        R"(
          begin identity_filter
          end
       )";

    // Parse
    const auto [tParseResult, tIter, tData] = parse_string(tInput);

    // Tests
    EXPECT_TRUE(tParseResult);
    EXPECT_EQ(tIter, tInput.end());
    ASSERT_TRUE(tData.mIdentityFilter);
    ASSERT_FALSE(tData.mIdentityFilter->filter_radius);
}

TEST(ParseIdentityFilterInputBlock, AllValidInputs)
{
    const std::string tInput =
        R"(
          begin identity_filter
            filter_radius 91.0
          end
       )";

    // Parse
    const auto [tParseResult, tIter, tData] = parse_string(tInput);

    // Tests
    EXPECT_TRUE(tParseResult);
    EXPECT_EQ(tIter, tInput.end());
    ASSERT_TRUE(tData.mIdentityFilter);
    test_existence_and_equality(tData.mIdentityFilter->filter_radius, 91.0);
}

TEST(ParseHelmholtzFilterInputBlock, AllValidInputs)
{
    const std::string tInput =
        R"(
          begin helmholtz_filter
            filter_radius 77.0
            boundary_sticking_penalty 86.0
          end
       )";

    // Parse
    const auto [tParseResult, tIter, tData] = parse_string(tInput);

    // Tests
    EXPECT_TRUE(tParseResult);
    EXPECT_EQ(tIter, tInput.end());
    ASSERT_TRUE(tData.mHelmholtzFilter);
    test_existence_and_equality(tData.mHelmholtzFilter->filter_radius, 77.0);
    test_existence_and_equality(tData.mHelmholtzFilter->boundary_sticking_penalty, 86.0);
}

TEST(ParseKernelFilterInputBlock, WrongCenteringType)
{
    const std::string tInput =
        R"(
          begin kernel_filter
            filter_radius 77.0
            centering_type edge
          end
       )";

    // Parse
    const auto [tParseResult, tIter, tData] = parse_string(tInput);
    // Tests
    EXPECT_FALSE(tParseResult);
    EXPECT_NE(tIter, tInput.cend());
}

TEST(ParseKernelFilterInputBlock, AllValidInputs)
{
    const std::string tInput =
        R"(
          begin kernel_filter
            filter_radius 17.0
            centering_type node
          end
       )";

    // Parse
    const auto [tParseResult, tIter, tData] = parse_string(tInput);

    // Tests
    EXPECT_TRUE(tParseResult);
    EXPECT_EQ(tIter, tInput.end());
    ASSERT_TRUE(tData.mKernelFilter);
    test_existence_and_equality(tData.mKernelFilter->filter_radius, 17.0);
    test_existence_and_equality(tData.mKernelFilter->centering_type, KernelFilterCenteringTypes::kNodeCentered);
}

}  // namespace plato::input_parser::unittest