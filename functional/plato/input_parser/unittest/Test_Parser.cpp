#include <gtest/gtest.h>

#include <boost/fusion/algorithm/iteration/for_each.hpp>
#include <type_traits>

#include "plato/input_parser/BlockStructRule.hpp"
#include "plato/input_parser/InputBlocks.hpp"
#include "plato/input_parser/InputParser.hpp"
#include "plato/input_parser/unittest/Test_Helpers.hpp"

namespace plato::input_parser::unittest
{
namespace
{
template <typename T>
constexpr bool kIsBoostOptional = false;

template <typename T>
constexpr bool kIsBoostOptional<boost::optional<T>> = true;

auto parse_string(const std::string& aInput) -> std::tuple<bool, std::string::const_iterator, ParsedInput>
{
    InputParser<std::string::const_iterator> tParser;
    ParsedInput tData;
    auto tIter = aInput.cbegin();
    const bool tParseResult = phrase_parse(tIter, aInput.cend(), tParser, boost::spirit::ascii::space, tData);
    return {tParseResult, tIter, tData};
}

void check_nothing_parsed(const ParsedInput& aInput)
{
    EXPECT_TRUE(aInput.mObjectives.empty());
    EXPECT_TRUE(aInput.mConstraints.empty());
    const auto check_value_empty = [](const auto& aValue)
    {
        static_assert(kIsBoostOptional<std::decay_t<decltype(aValue)>>,
                      "Only boost::optional values should be in an input struct");
        EXPECT_FALSE(aValue);
    };
    boost::fusion::for_each(aInput.mOptimizationParameters, check_value_empty);
}
}  // namespace

TEST(MassAppInput, ObjectiveAllValidInputs)
{
    const std::string tInput =
        R"(
          begin objective mp_objective
            active true
            app sierra_mass_app
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

    ASSERT_EQ(tData.mObjectives.size(), 1);
    const auto& tObjective = tData.mObjectives.front();
    test_existence_and_equality(tObjective.name, "mp_objective");
    test_existence_and_equality(tObjective.app, CodeOptions::kSierraMassApp);
    test_existence_and_equality(tObjective.shared_library_path, std::string{"/path/to/lib.so"});
    test_existence_and_equality(tObjective.number_of_processors, 10);
    test_existence_and_equality(tObjective.active, true);
    test_existence_and_equality(tObjective.input_files, std::vector<std::string>{"test.txt", "test2.xml"});
    test_existence_and_equality(tObjective.objective_type, ObjectiveTypes::kMinimize);
    test_existence_and_equality(tObjective.aggregation_weight, 10.0);
}

TEST(InputBlockStruct, GeometryBlocks)
{
    constexpr bool tDensityTopologyIsGeometry = kIsGeometryInput<density_topology>;
    EXPECT_TRUE(tDensityTopologyIsGeometry);
    constexpr bool tBrickShapeIsGeometry = kIsGeometryInput<brick_shape_geometry>;
    EXPECT_TRUE(tBrickShapeIsGeometry);
    constexpr bool tOptimizationIsNotGeometry = kIsGeometryInput<optimization_parameters>;
    EXPECT_FALSE(tOptimizationIsNotGeometry);
}

TEST(InputBlockStruct, BlockName)
{
    const std::string tResult = block_name<brick_shape_geometry>();
    EXPECT_EQ(tResult, "brick_shape_geometry");
}

TEST(MassAppInput, ConstraintAllValidInputs)
{
    const std::string tInput =
        R"(
          begin constraint mp_constraint
            active true
            app sierra_mass_app
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

    ASSERT_EQ(tData.mConstraints.size(), 1);
    const auto& tConstraint = tData.mConstraints.front();
    test_existence_and_equality(tConstraint.name, "mp_constraint");
    test_existence_and_equality(tConstraint.active, true);
    test_existence_and_equality(tConstraint.app, CodeOptions::kSierraMassApp);
    test_existence_and_equality(tConstraint.number_of_processors, 10);
    test_existence_and_equality(tConstraint.input_files, std::vector<std::string>{"test.txt"});
    test_existence_and_equality(tConstraint.equal_to, 1.0);
    test_existence_and_equality(tConstraint.is_linear, false);
}

TEST(MassAppInput, OptimizationParametersAllValidInputs)
{
    const std::string tInput =
        R"(
          begin optimization_parameters
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

    test_existence_and_equality(tData.mOptimizationParameters.input_file_name, std::string{"its-a_file.txt"});
    test_existence_and_equality(tData.mOptimizationParameters.max_iterations, 100u);
    test_existence_and_equality(tData.mOptimizationParameters.step_tolerance, 10.0);
    test_existence_and_equality(tData.mOptimizationParameters.gradient_tolerance, 100.0);
}

TEST(MassAppInput, ObjectiveNotAllInputs)
{
    const std::string tInput =
        R"(
          begin objective mp_objective
            app sierra_mass_app
            number_of_processors 10
            aggregation_weight 10.0
          end
       )";

    // Parse
    const auto [tParseResult, tIter, tData] = parse_string(tInput);

    ASSERT_EQ(tData.mObjectives.size(), 1);
    const auto& tObjective = tData.mObjectives.front();
    test_existence_and_equality(tObjective.name, "mp_objective");
    test_existence_and_equality(tObjective.app, CodeOptions::kSierraMassApp);
    test_existence_and_equality(tObjective.number_of_processors, 10);
    EXPECT_FALSE(tObjective.active);
    EXPECT_FALSE(tObjective.input_files);
    EXPECT_FALSE(tObjective.objective_type);
    test_existence_and_equality(tObjective.aggregation_weight, 10.0);
}

TEST(MassAppInput, MisspelledBegin)
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

TEST(MassAppInput, MisspelledEnd)
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

TEST(MassAppInput, MisspelledBlockType)
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

TEST(MassAppInput, MisspelledToken)
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

TEST(MassAppInput, MisspelledValue)
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

TEST(MassAppInput, MissingValue)
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
    EXPECT_EQ(tData.mConstraints.front().number_of_processors.value(), 10);
}

TEST(MassAppInput, ConstraintMultipleBlocks)
{
    const std::string tInput =
        R"(
          begin constraint mp_constraint_1
            active true
            app sierra_mass_app
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

    ASSERT_EQ(tData.mConstraints.size(), 2);
    const auto& tConstraint1 = tData.mConstraints.front();
    test_existence_and_equality(tConstraint1.name, "mp_constraint_1");
    test_existence_and_equality(tConstraint1.active, true);
    const auto& tConstraint2 = tData.mConstraints.back();
    test_existence_and_equality(tConstraint2.name, "mp_constraint_2");
    test_existence_and_equality(tConstraint2.number_of_processors, 10);
    test_existence_and_equality(tConstraint2.equal_to, -10.0);
}
}  // namespace plato::input_parser::unittest