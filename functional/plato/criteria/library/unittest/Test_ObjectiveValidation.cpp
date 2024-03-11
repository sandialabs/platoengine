#include <gtest/gtest.h>

#include "plato/criteria/library/CriterionValidation.hpp"
#include "plato/criteria/library/ObjectiveValidation.hpp"
#include "plato/input_parser/InputBlocks.hpp"
#include "plato/test_utilities/InputGeneration.hpp"

namespace plato::criteria::library::unittest
{
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
    namespace pfcd = plato::criteria::library::detail;

    // One objective and one rank
    input_parser::objective tObjective;
    EXPECT_FALSE(pfcd::validate_number_of_ranks_vs_objectives({tObjective}).has_value());
}

TEST(ObjectiveValidation, ErrorMessagesInvalidObjective)
{
    input_parser::objective tObjective = plato::test_utilities::create_valid_example_objective();
    tObjective.app = boost::none;
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
