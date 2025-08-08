#include <gtest/gtest.h>

#include <fstream>

#include "plato/input_parser/FileList.hpp"
#include "plato/input_parser/InputBlockStruct.hpp"
#include "plato/input_validation/ValidationUtilities.hpp"
#include "plato/test_utilities/FilesystemTestUtility.hpp"
#include "plato/test_utilities/TestContext.hpp"
#include "plato/utilities/Exception.hpp"

// clang-format off
PLATO_NAMED_INPUT_BLOCK_STRUCT(
    (plato)(input_parser), test_objective, plato::input_parser::ComponentType::kObjective,
    (bool, active, "")
    (double, aggregation_weight, "")
    (plato::input_parser::FileName, output_file_name, "")
)
// clang-format on

namespace plato::input_validation::unittest
{
TEST(ValidateUtilities, ActiveObjective)
{
    auto tObjectiveInput = input_parser::test_objective{};
    EXPECT_TRUE(is_active(tObjectiveInput));
    tObjectiveInput.active = true;
    EXPECT_TRUE(is_active(tObjectiveInput));
    tObjectiveInput.active = false;
    EXPECT_FALSE(is_active(tObjectiveInput));
}

TEST(ValidateUtilities, ValidateParameterExistsWhenParameterDoesNotExist)
{
    auto tObjectiveInput = input_parser::test_objective{};
    EXPECT_TRUE(
        error_message_for_empty_parameter("Objective: ", tObjectiveInput.aggregation_weight, "aggregation_weight")
            .has_value());
}

TEST(ValidateUtilities, ValidateParameterExistsDoesExist)
{
    auto tObjectiveInput = input_parser::test_objective{};
    tObjectiveInput.aggregation_weight = 23;
    EXPECT_FALSE(
        error_message_for_empty_parameter("Objective: ", tObjectiveInput.aggregation_weight, "aggregation_weight")
            .has_value());
}

TEST(ValidateUtilities, ValidateParameterWhenParameterDoesNotExistWithinBounds)
{
    namespace pfu = plato::utilities;
    auto tObjectiveInput = input_parser::test_objective{};
    EXPECT_TRUE(error_message_for_parameter_out_of_bounds("Objective: ", tObjectiveInput.aggregation_weight,
                                                          "aggregation_weight", pfu::unbounded<double>())
                    .has_value());
}

TEST(ValidateUtilities, ValidateParameterExistsWithinBounds)
{
    namespace pfu = plato::utilities;
    auto tObjectiveInput = input_parser::test_objective{};
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
    auto tObjectiveInput = input_parser::test_objective{};
    constexpr double tLowerBound = 0;

    tObjectiveInput.aggregation_weight = -23;
    EXPECT_TRUE(error_message_for_parameter_out_of_bounds("Objective: ", tObjectiveInput.aggregation_weight,
                                                          "aggregation_weight",
                                                          pfu::lower_bounded(pfu::Inclusive{tLowerBound}))
                    .has_value());
}

TEST(ValidationUtilities, ErrorMessageForMissingFileParameter)
{
    constexpr std::string_view tFileName = "dummy.txt";
    auto tObjectiveInput = input_parser::test_objective{};
    EXPECT_TRUE(
        error_message_for_missing_file_parameter("Objective: ", tObjectiveInput.output_file_name, "output_file_name")
            .has_value())
        << "Invalid: no file specified";

    tObjectiveInput.output_file_name = input_parser::FileName{std::string{tFileName}};
    EXPECT_TRUE(
        error_message_for_missing_file_parameter("Objective: ", tObjectiveInput.output_file_name, "output_file_name")
            .has_value())
        << "Invalid: file specified not on disk";

    [[maybe_unused]] std::ofstream tOutfile(std::string{tFileName});
    EXPECT_FALSE(
        error_message_for_missing_file_parameter("Objective: ", tObjectiveInput.output_file_name, "output_file_name")
            .has_value())
        << "Valid: file specified is on disk";
    test_utilities::test_for_existence_and_remove({tFileName},
                                                  TEST_CONTEXT("Verifying file creation worked for this test."));
}

}  // namespace plato::input_validation::unittest
