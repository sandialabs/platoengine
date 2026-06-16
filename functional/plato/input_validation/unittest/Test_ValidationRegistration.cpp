#include <gtest/gtest.h>

#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "plato/input_validation/ValidationRegistration.hpp"

namespace plato::input_validation::unittest
{
namespace
{
auto test_integer_function(const int /*unused*/) -> std::optional<std::string> { return std::nullopt; }
auto test_integer_function_two(const int /*unused*/) -> std::optional<std::string> { return "error message cruel int"; }
auto test_double_function(const double /*unused*/) -> std::optional<std::string>
{
    return "error message cruel double";
}
auto test_double_function_two(const double /*unused*/) -> std::optional<std::string> { return std::nullopt; }
auto test_double_function_int_argument(const double /*unused*/, const int /*unused*/) -> std::optional<std::string>
{
    return "error cruel int arg";
}

[[maybe_unused]] const auto kValidationRegistrationIntegers =
    ValidationRegistration<int>{[](const int aInput) { return test_integer_function(aInput); },
                                [](const int aInput) { return test_integer_function_two(aInput); }};

[[maybe_unused]] const auto kValidationRegistrationDoubles =
    ValidationRegistration<double>{[](const double aInput) { return test_double_function(aInput); }};

[[maybe_unused]] const auto kValidationRegistrationDoublesSecondIntentionalSplitForCTOR =
    ValidationRegistration<double>{[](const double aInput) { return test_double_function_two(aInput); }};

[[maybe_unused]] const auto kValidationRegistrationDoublesWithIntArgument = ValidationRegistration<double, int>{
    [](const double aInput, const int aArg) { return test_double_function_int_argument(aInput, aArg); }};

}  // namespace

TEST(Validation, NumberOfRegisteredFunctionsInt)
{
    const auto tRegisteredFunctions = detail::registered_validation_functions<int>();
    EXPECT_EQ(tRegisteredFunctions.size(), 2U);
}

TEST(Validation, NumberOfRegisteredFunctionsDouble)
{
    const auto tRegisteredFunctions = detail::registered_validation_functions<double>();
    EXPECT_EQ(tRegisteredFunctions.size(), 2U);
}

TEST(Validation, NumberOfRegisteredFunctionsDoubleWithIntArg)
{
    const auto tRegisteredFunctions = detail::registered_validation_functions<double, int>();
    EXPECT_EQ(tRegisteredFunctions.size(), 1U);
}

TEST(Validation, ValidateInputOnIntegers)
{
    std::vector<std::string> tMessages;
    constexpr auto tArbitraryValue = 666;
    tMessages = validate<int>(tArbitraryValue, std::move(tMessages));
    EXPECT_EQ(tMessages.size(), 1U);
}

TEST(Validation, ValidateInputOnDoubles)
{
    std::vector<std::string> tMessages;
    constexpr auto tArbitraryValue = 1.123;
    tMessages = validate<double>(tArbitraryValue, std::move(tMessages));
    EXPECT_EQ(tMessages.size(), 1U);
}

TEST(Validation, ValidateInputOnDoublesWithIntArgument)
{
    constexpr auto tArbitraryValue = 77.86;
    constexpr auto tArbitraryAdditionalArgument = 88;
    {
        const auto tMessages =
            validate<double, int>(tArbitraryValue, std::vector<std::string>{}, tArbitraryAdditionalArgument);
        EXPECT_EQ(tMessages.size(), 1U) << "Explicit types";
    }
    {
        const auto tMessages = validate(tArbitraryValue, std::vector<std::string>{}, tArbitraryAdditionalArgument);
        EXPECT_EQ(tMessages.size(), 1U) << "Deduced types";
    }
}

TEST(Validation, ValidateInputObjectiveList_EmptyNoneDefined)
{
    std::vector<std::string> tMessages;
    tMessages = validate<char>('c', std::move(tMessages));
    EXPECT_EQ(tMessages.size(), 0U);
}
}  // namespace plato::input_validation::unittest
