#ifndef PLATO_TEST_UTILITIES_VALIDATIONTESTUTILITIES
#define PLATO_TEST_UTILITIES_VALIDATIONTESTUTILITIES

#include <gtest/gtest.h>

#include <optional>
#include <string>

#include "plato/test_utilities/TestContext.hpp"

namespace plato::test_utilities
{
/// @brief Concept for validation functions. The main requirement is that they return a `std::optional<std::string>`.
template <typename Function, typename... Arguments>
concept ValidationFunction = std::is_invocable_r_v<std::optional<std::string>, Function, Arguments...>;

/// @brief Uses a gtest `EXPECT` macro to check that the function @a aValidationFunction, evaluated with arguments @a
/// aArguments, returns an empty optional, indicating that the input is valid.
template <typename... Arguments>
void expect_input_is_valid(const TestContext& aTestContext,
                           const ValidationFunction<Arguments...> auto& aValidationFunction,
                           Arguments&&... aArguments);

/// @brief Uses a gtest `EXPECT` macro to check that the function @a aValidationFunction, evaluated with arguments @a
/// aArguments, returns a non-empty optional, indicating that the input is not valid.
template <typename... Arguments>
void expect_input_is_invalid(const TestContext& aTestContext,
                             const ValidationFunction<Arguments...> auto& aValidationFunction,
                             Arguments&&... aArguments);

template <typename... Arguments>
void expect_input_is_valid(const TestContext& aTestContext,
                           const ValidationFunction<Arguments...> auto& aValidationFunction,
                           Arguments&&... aArguments)
{
    const auto tValidationMessage = aValidationFunction(std::forward<Arguments>(aArguments)...);
    EXPECT_FALSE(tValidationMessage.has_value())
        << aTestContext << "Expected no validation message, but got: " << tValidationMessage.value();
}

template <typename... Arguments>
void expect_input_is_invalid(const TestContext& aTestContext,
                             const ValidationFunction<Arguments...> auto& aValidationFunction,
                             Arguments&&... aArguments)
{
    const auto tValidationMessage = aValidationFunction(std::forward<Arguments>(aArguments)...);
    EXPECT_TRUE(tValidationMessage.has_value())
        << aTestContext << "Expected a validation message, but none was returned.";
}

}  // namespace plato::test_utilities

#endif
