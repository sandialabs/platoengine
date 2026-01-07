#include <gtest/gtest.h>

#include "plato/criteria/library/ObjectiveValidation.hpp"
#include "plato/criteria/library/test_utilities/ExampleInputBlocks.hpp"

namespace plato::criteria::library::unittest
{
TEST(ObjectiveValidation, ParallelObjectives)
{
    auto tInput = std::vector{test_utilities::create_valid_example_objective_input()};
    {
        // Check example, which sets number_of_processors to 1
        EXPECT_EQ(criteria::library::total_number_of_processors(tInput), 1u);
        EXPECT_FALSE(criteria::library::has_parallel_objective(tInput));
    }
    {
        // Set number_of_processors to boost::none, default is 1
        tInput.front().number_of_processors = boost::none;
        EXPECT_EQ(criteria::library::total_number_of_processors(tInput), 1u);
        EXPECT_FALSE(criteria::library::has_parallel_objective(tInput));
    }
    {
        // Set number_of_processors to 42
        tInput.front().number_of_processors = 42u;
        EXPECT_EQ(criteria::library::total_number_of_processors(tInput), 42u);
        EXPECT_TRUE(criteria::library::has_parallel_objective(tInput));
    }
    {
        // Add another objective with 1 processor
        tInput.push_back(test_utilities::create_valid_example_objective_input());
        EXPECT_EQ(criteria::library::total_number_of_processors(tInput), 43u);
        EXPECT_TRUE(criteria::library::has_parallel_objective(tInput));
    }
}

TEST(ObjectiveValidation, NumberOfActiveObjectives)
{
    auto tInput = std::vector{test_utilities::create_valid_example_objective_input()};
    // One active
    EXPECT_EQ(number_of_active_objectives(tInput), 1U);
    // boost::none as the default, should be active
    tInput.front().active = boost::none;
    EXPECT_EQ(number_of_active_objectives(tInput), 1U);
    // One inactive
    tInput.front().active = false;
    EXPECT_EQ(number_of_active_objectives(tInput), 0U);

    // Add an active objective
    tInput.push_back(test_utilities::create_valid_example_objective_input());
    EXPECT_EQ(number_of_active_objectives(tInput), 1U);

    // Reactivate the first
    tInput.front().active = true;
    EXPECT_EQ(number_of_active_objectives(tInput), 2U);
}

TEST(ObjectiveValidation, ValidateAggregationWeight)
{
    namespace pfcd = plato::criteria::library::detail;
    auto tObjective = input_parser::objective{};
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
    auto tObjective = input_parser::objective{};
    EXPECT_FALSE(pfcd::validate_at_least_one_objective({tObjective}).has_value());
    tObjective.active = false;
    EXPECT_TRUE(pfcd::validate_at_least_one_objective({tObjective}).has_value());
    EXPECT_TRUE(pfcd::validate_at_least_one_objective({tObjective, tObjective}).has_value());

    auto tObjectiveTwo = input_parser::objective{};
    tObjectiveTwo.active = true;
    EXPECT_FALSE(pfcd::validate_at_least_one_objective({tObjective, tObjectiveTwo}).has_value());
    EXPECT_FALSE(pfcd::validate_at_least_one_objective({tObjectiveTwo, tObjectiveTwo}).has_value());
}

TEST(ObjectiveValidation, ValidateMPIRanksVsNumberOfObjectives)
{
    // One objective and one rank
    auto tObjective = input_parser::objective{};
    EXPECT_FALSE(detail::validate_number_of_ranks_vs_serial_objectives({tObjective}).has_value());
    // Add an objective, should still be valid
    EXPECT_FALSE(detail::validate_number_of_ranks_vs_serial_objectives({tObjective, tObjective}).has_value());
}

TEST(ObjectiveValidation, ValidateMPIRanksVsNumberOfParallelObjectives)
{
    // No parallel objectives so this should not result in an error
    auto tObjective = input_parser::objective{};
    EXPECT_FALSE(detail::validate_number_of_ranks_vs_parallel_objectives({tObjective}).has_value());
    // Change to 2, should now be invalid
    tObjective.number_of_processors = 2u;
    EXPECT_TRUE(detail::validate_number_of_ranks_vs_parallel_objectives({tObjective}).has_value());
}
}  // namespace plato::criteria::library::unittest
