#include <gtest/gtest.h>

#include <exception>
#include <iostream>

#include "plato/core/ValidationRegistration.hpp"

namespace plato::core::unittest
{
namespace
{
std::optional<std::string> test_integer_function(const int) { return std::nullopt; }
std::optional<std::string> test_integer_function_two(const int) { return "error message cruel int"; }
std::optional<std::string> test_double_function(const double) { return "error message cruel double"; }
std::optional<std::string> test_double_function_two(const double) { return std::nullopt; }

[[maybe_unused]] static auto kValidationRegistrationIntegers =
    ValidationRegistration<int>{[](const int aInput) { return test_integer_function(aInput); },
                                [](const int aInput) { return test_integer_function_two(aInput); }};

[[maybe_unused]] static auto kValidationRegistrationDoubles =
    ValidationRegistration<double>{[](const double aInput) { return test_double_function(aInput); }};

[[maybe_unused]] static auto kValidationRegistrationDoublesSecondIntentionalSplitForCTOR =
    ValidationRegistration<double>{[](const double aInput) { return test_double_function_two(aInput); }};

}  // namespace

TEST(Validation, NumberOfRegisteredFunctionsInt)
{
    const auto tRegisteredFunctions = detail::registered_validation_functions<int>();
    EXPECT_EQ(tRegisteredFunctions.size(), 2u);
}

TEST(Validation, NumberOfRegisteredFunctionsDouble)
{
    const auto tRegisteredFunctions = detail::registered_validation_functions<double>();
    EXPECT_EQ(tRegisteredFunctions.size(), 2u);
}

TEST(Validation, ValidateInputOnIntegers)
{
    std::vector<std::string> tMessages;
    tMessages = validate<int>(666, std::move(tMessages));
    EXPECT_EQ(tMessages.size(), 1u);
}

TEST(Validation, ValidateInputOnDoubles)
{
    std::vector<std::string> tMessages;
    tMessages = validate<double>(1.123, std::move(tMessages));
    EXPECT_EQ(tMessages.size(), 1u);
}

TEST(Validation, ValidateInputObjectiveList_EmptyNoneDefined)
{
    std::vector<std::string> tMessages;
    tMessages = validate<char>('c', std::move(tMessages));
    EXPECT_EQ(tMessages.size(), 0u);
}
}  // namespace plato::core::unittest
