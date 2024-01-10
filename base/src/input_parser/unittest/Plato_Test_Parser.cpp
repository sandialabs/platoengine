#include <gtest/gtest.h>

#include "Plato_SuppressBoostNvccWarnings.hpp"

#include "Plato_InputBlocks.hpp"
#include "Plato_BlockStructRule.hpp"
#include "Plato_InputParser.hpp"
#include "Plato_Test_Helpers.hpp"

#include <boost/fusion/algorithm/iteration/for_each.hpp>

#include <type_traits>

namespace
{
template<typename T>
constexpr bool kIsBoostOptional = false;

template<typename T>
constexpr bool kIsBoostOptional<boost::optional<T>> = true;

auto parse_string(const std::string& aInput) -> std::tuple<bool, std::string::const_iterator, Plato::PlatoInput>
{
    Plato::InputParser<std::string::const_iterator> tParser;
    Plato::PlatoInput tData;
    auto tIter = aInput.cbegin();
    const bool tParseResult = phrase_parse(tIter, aInput.cend(), tParser, boost::spirit::ascii::space, tData);
    return {tParseResult, tIter, tData};
}

void check_nothing_parsed(const Plato::PlatoInput& aInput)
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
}

TEST(MassAppInput, ObjectiveAllValidInputs)
{
    const std::string tInput = 
       R"(
          begin objective mp_objective
            active true
            app sierra_mass_app
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
    Plato::Test::test_existence_and_equality(tObjective.name, "mp_objective");
    Plato::Test::test_existence_and_equality(tObjective.app, Plato::CodeOptions::kSierraMassApp);
    Plato::Test::test_existence_and_equality(tObjective.number_of_processors, 10);
    Plato::Test::test_existence_and_equality(tObjective.active, true);
    Plato::Test::test_existence_and_equality(
      tObjective.input_files, std::vector<std::string>{"test.txt", "test2.xml"});
    Plato::Test::test_existence_and_equality(tObjective.objective_type, Plato::ObjectiveTypes::kMinimize);
    Plato::Test::test_existence_and_equality(tObjective.aggregation_weight, 10.0);
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
            less_than 10.0
            greater_than -10.0
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
    Plato::Test::test_existence_and_equality(tConstraint.name, "mp_constraint");
    Plato::Test::test_existence_and_equality(tConstraint.active, true);
    Plato::Test::test_existence_and_equality(tConstraint.app, Plato::CodeOptions::kSierraMassApp);
    Plato::Test::test_existence_and_equality(tConstraint.number_of_processors, 10);
    Plato::Test::test_existence_and_equality(tConstraint.input_files, std::vector<std::string>{"test.txt"});
    Plato::Test::test_existence_and_equality(tConstraint.equal_to, 1.0);
    Plato::Test::test_existence_and_equality(tConstraint.less_than, 10.0);
    Plato::Test::test_existence_and_equality(tConstraint.greater_than, -10.0);
    Plato::Test::test_existence_and_equality(tConstraint.is_linear, false);
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

    Plato::Test::test_existence_and_equality(
      tData.mOptimizationParameters.input_file_name, std::string{"its-a_file.txt"});
    Plato::Test::test_existence_and_equality(
      tData.mOptimizationParameters.max_iterations, 100u);
    Plato::Test::test_existence_and_equality(
      tData.mOptimizationParameters.step_tolerance, 10.0);
    Plato::Test::test_existence_and_equality(
      tData.mOptimizationParameters.gradient_tolerance, 100.0);
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
    Plato::Test::test_existence_and_equality(tObjective.name, "mp_objective");
    Plato::Test::test_existence_and_equality(tObjective.app, Plato::CodeOptions::kSierraMassApp);
    Plato::Test::test_existence_and_equality(tObjective.number_of_processors, 10);
    EXPECT_FALSE(tObjective.active);
    EXPECT_FALSE(tObjective.input_files);
    EXPECT_FALSE(tObjective.objective_type);
    Plato::Test::test_existence_and_equality(tObjective.aggregation_weight, 10.0);
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
            greater_than -10.0
          end

       )";

    // Parse
    const auto [tParseResult, tIter, tData] = parse_string(tInput);
    
    // Tests
    EXPECT_TRUE(tParseResult);
    EXPECT_EQ(tIter, tInput.cend());

    ASSERT_EQ(tData.mConstraints.size(), 2);
    const auto& tConstraint1 = tData.mConstraints.front();
    Plato::Test::test_existence_and_equality(tConstraint1.name, "mp_constraint_1");
    Plato::Test::test_existence_and_equality(tConstraint1.active, true);
    const auto& tConstraint2 = tData.mConstraints.back();
    Plato::Test::test_existence_and_equality(tConstraint2.name, "mp_constraint_2");
    Plato::Test::test_existence_and_equality(tConstraint2.number_of_processors, 10);
    Plato::Test::test_existence_and_equality(tConstraint2.greater_than, -10.0);
}

#include "Plato_RestoreBoostNvccWarnings.hpp"
