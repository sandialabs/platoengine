#include <gtest/gtest.h>

#include <boost/fusion/algorithm/iteration/for_each.hpp>
#include <type_traits>

#include "plato/input_parser/BlockStructRule.hpp"
#include "plato/input_parser/InputBlocks.hpp"
#include "plato/input_parser/InputParser.hpp"
#include "plato/input_parser/unittest/Test_Helpers.hpp"
#include "plato/test_utilities/TestContext.hpp"

namespace plato::input_parser::unittest
{
namespace
{
auto parse_string(const std::string& aInput) -> std::tuple<bool, std::string::const_iterator, ParsedInput>
{
    using Iterator = std::string::const_iterator;
    InputParser<Iterator> tParser;
    ParsedInput tData;
    auto tIter = aInput.cbegin();
    const auto tSkipper = SkipperRule<Iterator>{};
    const bool tParseResult = phrase_parse(tIter, aInput.cend(), tParser, tSkipper.skipperRule(), tData);
    return {tParseResult, tIter, tData};
}

void check_nothing_parsed(const ParsedInput& aInput)
{
    EXPECT_TRUE(aInput.mObjectives.empty());
    EXPECT_TRUE(aInput.mConstraints.empty());
    EXPECT_FALSE(aInput.mROLOptimization);
}

ParsedInput parse_and_check_success(const std::string& aInput, const test_utilities::TestContext& aTestContext)
{
    const auto [tParseResult, tIter, tData] = parse_string(aInput);

    EXPECT_TRUE(tParseResult) << aTestContext;
    EXPECT_EQ(tIter, aInput.cend()) << aTestContext;
    return tData;
}

}  // namespace

TEST(ParsedInput, ObjectiveAllValidInputs)
{
    const std::string tInput =
        R"(
          begin objective mp_objective
            active true
            app volume
            shared_library_path /path/to/lib.so
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
    test_existence_and_equality(tObjective.app, CodeOptions::kVolume);
    test_existence_and_equality(tObjective.shared_library_path, std::string{"/path/to/lib.so"});
    test_existence_and_equality(tObjective.number_of_processors, 10u);
    test_existence_and_equality(tObjective.active, true);
    test_existence_and_equality(tObjective.input_files, std::vector<std::string>{"test.txt", "test2.xml"});
    test_existence_and_equality(tObjective.objective_type, ObjectiveTypes::kMinimize);
    test_existence_and_equality(tObjective.aggregation_weight, 10.0);
}

TEST(InputBlockStruct, GeometryBlocks)
{
    constexpr bool tDensityTopologyIsGeometry = IsGeometryInput<density_topology>::value;
    EXPECT_TRUE(tDensityTopologyIsGeometry);
    constexpr bool tBrickShapeIsGeometry = IsGeometryInput<brick_shape_geometry>::value;
    EXPECT_TRUE(tBrickShapeIsGeometry);
    constexpr bool tOptimizationIsNotGeometry = IsGeometryInput<rol_optimization>::value;
    EXPECT_FALSE(tOptimizationIsNotGeometry);
}

TEST(InputBlockStruct, BlockName)
{
    const std::string tResult = block_name<brick_shape_geometry>();
    EXPECT_EQ(tResult, "brick_shape_geometry");
}

TEST(ParsedInput, ConstraintAllValidInputs)
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
    test_existence_and_equality(tConstraint.app, CodeOptions::kVolumeFraction);
    test_existence_and_equality(tConstraint.number_of_processors, 10u);
    test_existence_and_equality(tConstraint.input_files, std::vector<std::string>{"test.txt"});
    test_existence_and_equality(tConstraint.equal_to, 1.0);
    test_existence_and_equality(tConstraint.is_linear, false);
}

TEST(ParsedInput, OptimizationParametersAllValidInputs)
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

TEST(ParsedInput, GradientCheckAllValidInputs)
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

TEST(ParsedInput, ConstraintCheckAllValidInputs)
{
    const std::string tInput =
        R"(
          begin constraint_check
            linearity_check_output_file_name ROL_constraint_linearity_check_output.txt
            jacobian_check_output_file_name ROL_constraint_jacobian_check_output.txt
            jacobian_adjoint_consistency_output_file_name ROL_constraint_jacobian_adjoint_consistency_check_output.txt
            number_of_steps 10
            initial_direction_magnitude 1
            step_size_reduction_factor 0.1
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

TEST(ParsedInput, ObjectiveNotAllInputs)
{
    const std::string tInput =
        R"(
          begin objective mp_objective
            app nodal_sum
            number_of_processors 10
            aggregation_weight 10.0
          end
       )";

    // Parse
    const auto [tParseResult, tIter, tData] = parse_string(tInput);

    ASSERT_EQ(tData.mObjectives.size(), 1u);
    const auto& tObjective = tData.mObjectives.front();
    test_existence_and_equality(tObjective.name, "mp_objective");
    test_existence_and_equality(tObjective.app, CodeOptions::kNodalSum);
    test_existence_and_equality(tObjective.number_of_processors, 10u);
    EXPECT_FALSE(tObjective.active);
    EXPECT_FALSE(tObjective.input_files);
    EXPECT_FALSE(tObjective.objective_type);
    test_existence_and_equality(tObjective.aggregation_weight, 10.0);
}

TEST(ParsedInput, MisspelledBegin)
{
    const std::string tInput =
        R"(
          begn constraint mp_constraint
            active true
          end
       )";

    // Parse
    const auto [tParseResult, tIter, tData] = parse_string(tInput);

    // Tests
    EXPECT_TRUE(tParseResult);
    EXPECT_NE(tIter, tInput.cend());
    check_nothing_parsed(tData);
}

TEST(ParsedInput, MisspelledEnd)
{
    const std::string tInput =
        R"(
          begin constraint mp_constraint
            active true
          en
       )";

    // Parse
    const auto [tParseResult, tIter, tData] = parse_string(tInput);

    // Tests
    EXPECT_FALSE(tParseResult);
    EXPECT_NE(tIter, tInput.cend());
    check_nothing_parsed(tData);
}

TEST(ParsedInput, MisspelledBlockType)
{
    const std::string tInput =
        R"(
          begin constrain mp_constraint
            active true
          end
       )";

    // Parse
    const auto [tParseResult, tIter, tData] = parse_string(tInput);

    // Tests
    EXPECT_TRUE(tParseResult);
    EXPECT_NE(tIter, tInput.cend());
    check_nothing_parsed(tData);
}

TEST(ParsedInput, MisspelledToken)
{
    const std::string tInput =
        R"(
          begin constraint mp_constraint
            ave true
          end
       )";

    // Parse
    const auto [tParseResult, tIter, tData] = parse_string(tInput);

    // Tests
    EXPECT_FALSE(tParseResult);
    EXPECT_NE(tIter, tInput.cend());
    check_nothing_parsed(tData);
}

TEST(ParsedInput, MisspelledValue)
{
    const std::string tInput =
        R"(
          begin constraint mp_constraint
            active te
          end
       )";

    // Parse
    const auto [tParseResult, tIter, tData] = parse_string(tInput);

    // Tests
    EXPECT_FALSE(tParseResult);
    EXPECT_NE(tIter, tInput.cend());
    check_nothing_parsed(tData);
}

TEST(ParsedInput, MissingValue)
{
    const std::string tInput =
        R"(
          begin constraint mp_constraint
            number_of_processors 10
            active
          end
       )";

    // Parse
    const auto [tParseResult, tIter, tData] = parse_string(tInput);

    // Tests
    EXPECT_FALSE(tData.mConstraints.front().active);
    EXPECT_TRUE(tData.mConstraints.front().number_of_processors);
    EXPECT_EQ(tData.mConstraints.front().number_of_processors.value(), 10u);
}

TEST(ParsedInput, ConstraintMultipleBlocks)
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

TEST(ParsedInput, CommentWithinLine)
{
    const std::string tInput =
        R"(
          begin objective objective_1 
            active false # true
            app nodal_sum
          end
       )";

    const auto tParsedInput = parse_and_check_success(tInput, TEST_CONTEXT("Commented out input mid-line"));

    ASSERT_EQ(tParsedInput.mObjectives.size(), 1u);
    test_existence_and_equality(tParsedInput.mObjectives.front().active, false);
    test_existence_and_equality(tParsedInput.mObjectives.front().app, CodeOptions::kNodalSum);
}

TEST(ParsedInput, CommentEntireLine)
{
    const std::string tInput =
        R"(
          begin objective objective_1 
            # app nodal_sum
            app volume
          end
       )";

    const auto tParsedInput = parse_and_check_success(tInput, TEST_CONTEXT("Commented out input"));

    ASSERT_EQ(tParsedInput.mObjectives.size(), 1u);
    test_existence_and_equality(tParsedInput.mObjectives.front().app, CodeOptions::kVolume);
}

TEST(ParsedInput, CommentNonInput)
{
    const std::string tInput =
        R"(
          begin objective objective_1 
            # This is not real input!
            app volume
          end
       )";
    const auto tParsedInput = parse_and_check_success(tInput, TEST_CONTEXT("Comment on non-input"));

    ASSERT_EQ(tParsedInput.mObjectives.size(), 1u);
    test_existence_and_equality(tParsedInput.mObjectives.front().app, CodeOptions::kVolume);
}

TEST(ParsedInput, CommentMultipleLinesAndCharacters)
{
    const std::string tInput =
        R"(
          begin objective objective_1 
            ## This is not # real input!
            # active false
            app volume
          end
       )";
    const auto tParsedInput = parse_and_check_success(tInput, TEST_CONTEXT("Comment multiple lines and characters"));

    ASSERT_EQ(tParsedInput.mObjectives.size(), 1u);
    test_existence_and_equality(tParsedInput.mObjectives.front().app, CodeOptions::kVolume);
    EXPECT_FALSE(tParsedInput.mObjectives.front().active.has_value());
}

}  // namespace plato::input_parser::unittest
