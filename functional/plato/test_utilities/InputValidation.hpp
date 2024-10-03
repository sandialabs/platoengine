#ifndef PLATO_TEST_UTILITIES_INPUTVALIDATION
#define PLATO_TEST_UTILITIES_INPUTVALIDATION

#include <gtest/gtest.h>

#include <functional>
#include <optional>
#include <string>

#include "plato/test_utilities/TestContext.hpp"

namespace plato::test_utilities
{

/// @brief A templated function that will call the input generator function specified @a aValidInputGenerationFunction
/// that is assumed to be valid and run specified test @a aTestFunction
/// The second test default constructs the InputBlock and runs the test again with the expected result of @a
/// aEmptyParameterGold
template <typename InputBlock>
void test_validation_function_using_valid_function_generator_vs_empty_struct(
    std::function<std::optional<std::string>(const InputBlock&)> aTestFunction,
    std::function<InputBlock()> aValidInputGenerationFunction,
    const bool aEmptyParameterGold,
    const test_utilities::TestContext& aTestContext)
{
    {
        constexpr bool tValidParameterGold = false;
        const auto tInput = aValidInputGenerationFunction();
        const auto tResult = aTestFunction(tInput);
        EXPECT_EQ(tResult.has_value(), tValidParameterGold) << aTestContext;
    }
    {
        const InputBlock tInput{};
        const auto tResult = aTestFunction(tInput);
        EXPECT_EQ(tResult.has_value(), aEmptyParameterGold) << aTestContext;
    }
}

}  // namespace plato::test_utilities

#endif
