#include <gtest/gtest.h>

#include "plato/criteria/library/CriterionValidation.hpp"
#include "plato/criteria/library/ObjectiveValidation.hpp"
#include "plato/input_parser/InputBlocks.hpp"
#include "plato/test_utilities/InputGeneration.hpp"

namespace plato::criteria::library::unittest
{
TEST(ObjectiveValidation, ParallelObjectives)
{
    auto tInput = test_utilities::create_valid_example_input();
    {
        // Check example, which sets number_of_processors to 1
        EXPECT_EQ(criteria::library::total_number_of_processors(tInput.mObjectives), 1u);
        EXPECT_FALSE(criteria::library::has_parallel_objective(tInput.mObjectives));
    }
    {
        // Set number_of_processors to boost::none, default is 1
        tInput.mObjectives.front().number_of_processors = boost::none;
        EXPECT_EQ(criteria::library::total_number_of_processors(tInput.mObjectives), 1u);
        EXPECT_FALSE(criteria::library::has_parallel_objective(tInput.mObjectives));
    }
    {
        // Set number_of_processors to 42
        tInput.mObjectives.front().number_of_processors = 42u;
        EXPECT_EQ(criteria::library::total_number_of_processors(tInput.mObjectives), 42u);
        EXPECT_TRUE(criteria::library::has_parallel_objective(tInput.mObjectives));
    }
    {
        // Add another objective with 1 processor
        tInput.mObjectives.push_back(test_utilities::create_valid_example_objective());
        EXPECT_EQ(criteria::library::total_number_of_processors(tInput.mObjectives), 43u);
        EXPECT_TRUE(criteria::library::has_parallel_objective(tInput.mObjectives));
    }
}

TEST(ObjectiveValidation, ValidateAggregationWeight)
{
    namespace pfcd = plato::criteria::library::detail;
    input_parser::objective tObjective;
    EXPECT_TRUE(pfcd::validate_aggregation_weight(tObjective).has_value());
    tObjective.aggregation_weight = 13.0;
    EXPECT_FALSE(pfcd::validate_aggregation_weight(tObjective).has_value());
    tObjective.aggregation_weight = -13.0;
    EXPECT_TRUE(pfcd::validate_aggregation_weight(tObjective).has_value());
}

TEST(ObjectiveValidation, ValidateAtLeastOneObjective)
{
    namespace pfcd = plato::criteria::library::detail;
    EXPECT_TRUE(pfcd::validate_at_least_one_objective({}).has_value());
    input_parser::objective tObjective;
    EXPECT_FALSE(pfcd::validate_at_least_one_objective({tObjective}).has_value());
    tObjective.active = false;
    EXPECT_TRUE(pfcd::validate_at_least_one_objective({tObjective}).has_value());
    EXPECT_TRUE(pfcd::validate_at_least_one_objective({tObjective, tObjective}).has_value());

    input_parser::objective tObjectiveTwo;
    tObjectiveTwo.active = true;
    EXPECT_FALSE(pfcd::validate_at_least_one_objective({tObjective, tObjectiveTwo}).has_value());
    EXPECT_FALSE(pfcd::validate_at_least_one_objective({tObjectiveTwo, tObjectiveTwo}).has_value());
}

TEST(ObjectiveValidation, ValidateMPIRanksVsNumberOfObjectives)
{
    // One objective and one rank
    input_parser::objective tObjective;
    EXPECT_FALSE(detail::validate_number_of_ranks_vs_serial_objectives({tObjective}).has_value());
    // Add an objective, should still be valid
    EXPECT_FALSE(detail::validate_number_of_ranks_vs_serial_objectives({tObjective, tObjective}).has_value());
}

TEST(ObjectiveValidation, ValidateMPIRanksVsNumberOfParallelObjectives)
{
    // No parallel objectives so this should not result in an error
    input_parser::objective tObjective;
    EXPECT_FALSE(detail::validate_number_of_ranks_vs_parallel_objectives({tObjective}).has_value());
    // Change to 2, should now be invalid
    tObjective.number_of_processors = 2u;
    EXPECT_TRUE(detail::validate_number_of_ranks_vs_parallel_objectives({tObjective}).has_value());
}

TEST(ObjectiveValidation, ErrorMessagesInvalidObjective)
{
    input_parser::objective tObjective = plato::test_utilities::create_valid_example_objective();
    tObjective.criterion = boost::none;
    std::vector<std::string> tMessages;
    tMessages = core::validate(tObjective, std::move(tMessages));
    EXPECT_EQ(tMessages.size(), 1u);
}

TEST(ObjectiveValidation, ErrorMessagesInvalidInput)
{
    namespace pfc = plato::criteria::library;
    const input_parser::objective tObjective;
    const std::vector<input_parser::objective> tInput{tObjective, tObjective};

    std::vector<std::string> tMessages;
    tMessages = pfc::validate_objectives(tInput, std::move(tMessages));
    EXPECT_EQ(tMessages.size(), 4u);
}

TEST(ObjectiveValidation, NoErrorMessagesValidObjective)
{
    namespace pfc = plato::criteria::library;
    const auto tObjective = plato::test_utilities::create_valid_example_objective();
    const std::vector<input_parser::objective> tInput{tObjective, tObjective};

    std::vector<std::string> tMessages;
    tMessages = pfc::validate_objectives(tInput, std::move(tMessages));
    EXPECT_EQ(tMessages.size(), 0u);
}

TEST(ObjectiveValidation, ErrorMessagesInvalidObjectives)
{
    namespace pfc = plato::criteria::library;
    auto tObjective = plato::test_utilities::create_valid_example_objective();
    tObjective.active = false;
    const std::vector<input_parser::objective> tInput{tObjective, tObjective};

    std::vector<std::string> tMessages;
    tMessages = pfc::validate_objectives(tInput, std::move(tMessages));
    EXPECT_EQ(tMessages.size(), 1u);
}
}  // namespace plato::criteria::library::unittest
