#include <gtest/gtest.h>

#include "plato/core/ValidationUtilities.hpp"
#include "plato/process_manager/extension/GradientCheck.hpp"
#include "plato/process_manager/library/ValidatedInput.hpp"
#include "plato/test_utilities/InputGeneration.hpp"
#include "plato/utilities/Exception.hpp"

namespace plato::process_manager::extension::unittest
{
namespace
{
std::vector<std::string> validate_gradient_check(const input_parser::gradient_check& aInput,
                                                 std::vector<std::string>&& aCurrentMessageList)
{
    return core::validate(aInput, std::move(aCurrentMessageList));
}

}  // namespace

TEST(ValidateGradientCheck, ValidateOuputFileName)
{
    auto tGradientCheck = input_parser::gradient_check{};
    EXPECT_TRUE(detail::validate_output_file_name(tGradientCheck).has_value());
    tGradientCheck.output_file_name = input_parser::FileName{"file.txt"};  // Requires an input
    EXPECT_FALSE(detail::validate_output_file_name(tGradientCheck).has_value());
}

TEST(ValidateGradientCheck, NoErrorMessagesValidGradientCheck)
{
    const input_parser::gradient_check tGradientCheck = plato::test_utilities::create_valid_example_gradient_check();
    const auto tMessages = validate_gradient_check(tGradientCheck, std::vector<std::string>{});
    EXPECT_TRUE(tMessages.empty());
}

TEST(ValidateGradientCheck, ErrorMessagesInvalidGradientCheck)
{
    const input_parser::gradient_check tGradientCheck;
    const auto tMessages = validate_gradient_check(tGradientCheck, std::vector<std::string>{});
    const auto tNumberOfGradientCheckValidationFunctions =
        core::detail::registered_validation_functions<input_parser::gradient_check>().size();
    EXPECT_EQ(tMessages.size(), tNumberOfGradientCheckValidationFunctions);
}

}  // namespace plato::process_manager::extension::unittest