#include <gtest/gtest.h>

#include "plato/input_validation/ValidationFunction.hpp"

namespace plato::input_validation::unittest
{
namespace
{
struct ExcellentInput
{
    int field1 = 42;
};

struct BogusInput
{
};

constexpr auto kErrorMessage = std::string_view{"Wrong!"};

constexpr auto kCheckForFortyTwo = [](const ExcellentInput& aInput) -> std::optional<std::string>
{
    if (aInput.field1 != 42)
    {
        return std::string{kErrorMessage};
    }
    return std::nullopt;
};

constexpr auto kCheckForSumFortyOne = [](const ExcellentInput& aInput,
                                         const int aAnotherInt) -> std::optional<std::string>
{
    if (aInput.field1 + aAnotherInt != 41)
    {
        return std::optional{std::string{kErrorMessage}};
    }
    return std::nullopt;
};

}  // namespace

TEST(ValidationFunction, ValidateInputBlockWrapper)
{
    const auto tValidationFunction = ValidationFunction<input_parser::InputBlockWrapper>{kCheckForFortyTwo};

    {
        const auto tInput = input_parser::InputBlockWrapper{ExcellentInput{/*.field1=*/43}};
        const auto tResult = tValidationFunction.validate(tInput);
        ASSERT_TRUE(tResult.has_value());
        EXPECT_EQ(tResult.value(), kErrorMessage);
    }
    {
        const auto tInput = input_parser::InputBlockWrapper{ExcellentInput{/*.field1=*/42}};
        ASSERT_FALSE(tValidationFunction.validate(tInput).has_value());
    }
}

TEST(ValidationFunction, ValidateInputBlockWrapperAdditionalArgs)
{
    const auto tValidationFunction = ValidationFunction<input_parser::InputBlockWrapper, int>{kCheckForSumFortyOne};
    const auto tInput = input_parser::InputBlockWrapper{ExcellentInput{/*.field1=*/41}};

    // Invalid input
    const auto tResult = tValidationFunction.validate(tInput, 1);
    ASSERT_TRUE(tResult.has_value());
    EXPECT_EQ(tResult.value(), kErrorMessage);

    // Valid input
    ASSERT_FALSE(tValidationFunction.validate(tInput, 0).has_value());
}

TEST(ValidationFunction, ValidateWrongType)
{
    const auto tValidationFunction = ValidationFunction<input_parser::InputBlockWrapper>{
        [](const ExcellentInput&) -> std::optional<std::string> { return std::string{"Error!"}; }};

    const auto tInput = input_parser::InputBlockWrapper{BogusInput{}};
    const auto tResult = tValidationFunction.validate(tInput);
    EXPECT_FALSE(tResult.has_value()) << tResult.value();
}

TEST(ValidationFunction, ValidateArbitraryInput)
{
    const auto tValidationFunction = ValidationFunction<ExcellentInput>{kCheckForFortyTwo};
    const auto tResult = tValidationFunction.validate(ExcellentInput{/*.field1=*/43});
    ASSERT_TRUE(tResult.has_value());
    EXPECT_EQ(tResult.value(), kErrorMessage);
}

TEST(ValidationFunction, ValidateInputAdditionalArgs)
{
    const auto tValidationFunction = ValidationFunction<ExcellentInput, int>{kCheckForSumFortyOne};

    // Valid input
    EXPECT_FALSE(tValidationFunction.validate(ExcellentInput{/*.field1=*/41}, 0).has_value());

    // Invalid input
    const auto tResult = tValidationFunction.validate(ExcellentInput{/*.field1=*/41}, 1);
    ASSERT_TRUE(tResult.has_value());
    EXPECT_EQ(tResult.value(), kErrorMessage);
}
}  // namespace plato::input_validation::unittest
