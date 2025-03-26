#include <gtest/gtest.h>

#include "plato/core/ValidationFunction.hpp"

namespace plato::core::unittest
{
namespace
{
struct BogusInput
{
    int field1 = 42;
};
}  // namespace

TEST(ValidationFunctionWrapper, Validate)
{
    constexpr auto tErrorMessage = std::string_view{"Wrong!"};
    const auto tValidationFunction =
        ValidationFunction{[tErrorMessage](const BogusInput& aInput) -> std::optional<std::string>
                           {
                               if (aInput.field1 != 42)
                               {
                                   return std::string{tErrorMessage};
                               }
                               return std::nullopt;
                           }};

    auto tInput = input_parser::CrossReferencedInput{};
    {
        tInput.set(BogusInput{/*.field1=*/43});
        const auto tResult = tValidationFunction.validate(tInput);
        ASSERT_TRUE(tResult.has_value());
        EXPECT_EQ(tResult.value(), tErrorMessage);
    }
    {
        tInput.set(BogusInput{});
        const auto tResult = tValidationFunction.validate(tInput);
        ASSERT_FALSE(tResult.has_value());
    }
}
}  // namespace plato::core::unittest
