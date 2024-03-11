#include <gtest/gtest.h>

#include "plato/core/ValidationUtilities.hpp"
#include "plato/utilities/Exception.hpp"

namespace plato::core::unittest
{
TEST(ValidateUtilities, ActiveConstraint)
{
    input_parser::constraint tConstraintInput;
    EXPECT_TRUE(is_active(tConstraintInput));
    tConstraintInput.active = true;
    EXPECT_TRUE(is_active(tConstraintInput));
    tConstraintInput.active = false;
    EXPECT_FALSE(is_active(tConstraintInput));
}

TEST(ValidateUtilities, ActiveObjective)
{
    input_parser::objective tObjectiveInput;
    EXPECT_TRUE(is_active(tObjectiveInput));
    tObjectiveInput.active = true;
    EXPECT_TRUE(is_active(tObjectiveInput));
    tObjectiveInput.active = false;
    EXPECT_FALSE(is_active(tObjectiveInput));
}

TEST(ValidateUtilities, ValidateParameterExistsWhenParameterDoesNotExist)
{
    input_parser::objective tObjectiveInput;
    EXPECT_TRUE(
        error_message_for_empty_parameter("Objective: ", tObjectiveInput.aggregation_weight, "aggregation_weight")
            .has_value());
}

TEST(ValidateUtilities, ValidateParameterExistsDoesExist)
{
    input_parser::objective tObjectiveInput;
    tObjectiveInput.aggregation_weight = 23;
    EXPECT_FALSE(
        error_message_for_empty_parameter("Objective: ", tObjectiveInput.aggregation_weight, "aggregation_weight")
            .has_value());
}

TEST(ValidateUtilities, ValidateParameterWhenParameterDoesNotExistWithinBounds)
{
    namespace pfu = plato::utilities;
    input_parser::objective tObjectiveInput;
    EXPECT_TRUE(error_message_for_parameter_out_of_bounds("Objective: ", tObjectiveInput.aggregation_weight,
                                                          "aggregation_weight", pfu::unbounded<double>())
                    .has_value());
}

TEST(ValidateUtilities, ValidateParameterExistsWithinBounds)
{
    namespace pfu = plato::utilities;
    input_parser::objective tObjectiveInput;
    constexpr double tLowerBound = 0;

    tObjectiveInput.aggregation_weight = 23;
    EXPECT_FALSE(error_message_for_parameter_out_of_bounds("Objective: ", tObjectiveInput.aggregation_weight,
                                                           "aggregation_weight",
                                                           pfu::lower_bounded(pfu::Inclusive{tLowerBound}))
                     .has_value());
}

TEST(ValidateUtilities, ValidateParameterExistsOutOfBounds)
{
    namespace pfu = plato::utilities;
    input_parser::objective tObjectiveInput;
    constexpr double tLowerBound = 0;

    tObjectiveInput.aggregation_weight = -23;
    EXPECT_TRUE(error_message_for_parameter_out_of_bounds("Objective: ", tObjectiveInput.aggregation_weight,
                                                          "aggregation_weight",
                                                          pfu::lower_bounded(pfu::Inclusive{tLowerBound}))
                    .has_value());
}

TEST(ValidateUtilities, AllMessages)
{
    const auto tMessage1 = std::string{"one fish"};
    const auto tMessage2 = std::string{"two fish"};
    const auto tMessages = std::vector{tMessage1, tMessage2};
    const auto tAllMessages = all_messages(tMessages);
    const auto tExpected = tMessage1 + "\n" + tMessage2 + "\n";
    EXPECT_EQ(tExpected, tAllMessages);
}
}  // namespace plato::core::unittest
