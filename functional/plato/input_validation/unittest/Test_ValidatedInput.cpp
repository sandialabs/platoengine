#include <gtest/gtest.h>

#include "plato/input_parser/ComponentBlockParser.hpp"
#include "plato/input_parser/CrossLinkedInput.hpp"
#include "plato/input_parser/InputBlockStruct.hpp"
#include "plato/input_parser/ParsedInput.hpp"
#include "plato/input_validation/ValidatedInput.hpp"
#include "plato/input_validation/ValidationRegistration.hpp"

// Define input structs
// clang-format off
PLATO_INPUT_BLOCK_STRUCT((plato)(input_parser),
                         marvel, input_parser::ComponentType::kGeometry,
                         (double, cyclops, "")
                         (int, wolverine, ""))

PLATO_NAMED_INPUT_BLOCK_STRUCT((plato)(input_parser),
                         dc, plato::input_parser::ComponentType::kConstraint,
                         (bool, superman, "")
                         (unsigned int, batman, ""))
// clang-format on

namespace plato::input_validation::unittest
{
namespace
{
constexpr auto kMarvelValidationErrorMessage = std::string_view{"Marvel error!"};
constexpr auto kDCValidationErrorMessage = std::string_view{"DC error!"};
constexpr auto kParsedInputValidationErrorMessage = std::string_view{"Multiverse error!"};

const auto kValidMarvel = input_parser::marvel{/*.cyclops=*/42.0, /*.wolverine=*/100};
const auto kValidDC = input_parser::dc{/*.name=*/std::string{"tv-show"}, /*.superman=*/true, /*.batman=*/100U};

[[nodiscard]] auto validate_wolverine(const boost::optional<int> aWolverine) -> std::optional<std::string>
{
    if (aWolverine.has_value() && aWolverine != 100)
    {
        return std::string{kMarvelValidationErrorMessage};
    }
    return std::nullopt;
}

[[nodiscard]] auto validate_superman(const boost::optional<bool> aSuperman) -> std::optional<std::string>
{
    if (!aSuperman.has_value())
    {
        return std::string{kDCValidationErrorMessage};
    }
    return std::nullopt;
}

[[nodiscard]] auto validate_parsed_input(const input_parser::NewParsedInput& aParsedInput) -> std::optional<std::string>
{
    if (aParsedInput.get<input_parser::ComponentType::kGeometry>().empty())
    {
        return std::string{kParsedInputValidationErrorMessage};
    }
    return std::nullopt;
}

[[maybe_unused]] static auto kBlockValidationRegistration = CrossReferencedInputValidationRegistration<>{
    {[](const input_parser::marvel& aInput) { return validate_wolverine(aInput.wolverine); },
     [](const input_parser::dc& aInput) { return validate_superman(aInput.superman); }}};

[[maybe_unused]] static auto kInputValidationRegistration = NewParsedInputValidationRegistration<>{{
    [](const input_parser::NewParsedInput& aInput) { return validate_parsed_input(aInput); },
}};

auto make_test_input(const std::optional<input_parser::marvel>& aMarvelInput,
                     const std::optional<input_parser::dc>& aDCInput) -> input_parser::CrossLinkedInput
{
    auto tInputs = std::vector<input_parser::InputDataBlock>{};
    if (aMarvelInput)
    {
        tInputs.push_back(input_parser::InputDataBlock{input_parser::ComponentType::kGeometry, "marvel",
                                                       input_parser::CrossReferencedInput{aMarvelInput.value()}});
    }
    if (aDCInput)
    {
        tInputs.push_back(input_parser::InputDataBlock{input_parser::ComponentType::kConstraint, "dc",
                                                       input_parser::CrossReferencedInput{aDCInput.value()}});
    }
    const auto tCrossLinkedInput =
        input_parser::make_cross_linked_input(input_parser::NewParsedInput{std::move(tInputs)});
    EXPECT_TRUE(tCrossLinkedInput.hasValue());
    return tCrossLinkedInput.value();
}

}  // namespace

TEST(ValidatedInput, ConstructionValidInput)
{
    const auto tCrossLinkedInput = make_test_input(kValidMarvel, kValidDC);
    const auto tValidatedInput = make_validated_input(tCrossLinkedInput);
    ASSERT_TRUE(tValidatedInput.hasValue());
}

TEST(ValidatedInput, ConstructionOneEntryInvalidInput)
{
    const auto kInvalidMarvel = input_parser::marvel{/*.cyclops=*/42.0, /*.wolverine=*/101};
    const auto tCrossLinkedInput = make_test_input(kInvalidMarvel, std::nullopt);
    const auto tValidatedInput = make_validated_input(tCrossLinkedInput);

    ASSERT_TRUE(tValidatedInput.hasError());
    EXPECT_EQ(tValidatedInput.error(), kMarvelValidationErrorMessage);
}

TEST(ValidatedInput, ConstructionTwoEntriesInvalidInput)
{
    const auto kInvalidMarvel = input_parser::marvel{/*.cyclops=*/42.0, /*.wolverine=*/101};
    const auto kInvalidDC =
        input_parser::dc{/*.name=*/std::string{"tv-show"}, /*.superman=*/boost::none, /*.batman=*/100U};
    const auto tCrossLinkedInput = make_test_input(kInvalidMarvel, kInvalidDC);
    const auto tValidatedInput = make_validated_input(tCrossLinkedInput);

    ASSERT_TRUE(tValidatedInput.hasError());
    EXPECT_EQ(tValidatedInput.error(),
              std::string{kDCValidationErrorMessage} + "\n" + std::string{kMarvelValidationErrorMessage});
}

TEST(ValidatedInput, ConstructionInvalidParsedInput)
{
    const auto tCrossLinkedInput = make_test_input(std::nullopt, kValidDC);
    const auto tValidatedInput = make_validated_input(tCrossLinkedInput);

    ASSERT_TRUE(tValidatedInput.hasError());
    EXPECT_EQ(tValidatedInput.error(), std::string{kParsedInputValidationErrorMessage});
}

TEST(ValidatedInput, GetMember)
{
    const auto tCrossLinkedInput = make_test_input(kValidMarvel, kValidDC);
    const auto tValidatedInputOrError = make_validated_input(tCrossLinkedInput);

    ASSERT_TRUE(tValidatedInputOrError.hasValue());
    const auto& tValidatedInput = tValidatedInputOrError.value();

    const auto tGeometryInput = tValidatedInput.get<input_parser::ComponentType::kGeometry>().rawInput();
    EXPECT_EQ(tGeometryInput.mComponentType, input_parser::ComponentType::kGeometry);
    EXPECT_EQ(tGeometryInput.mBlockName, "marvel");
    ASSERT_TRUE(tGeometryInput.mInput.holds_expected_type<input_parser::marvel>());
    ASSERT_TRUE(tGeometryInput.mInput.get<input_parser::marvel>().cyclops.has_value());
    EXPECT_EQ(tGeometryInput.mInput.get<input_parser::marvel>().cyclops.value(), 42.0);
    ASSERT_TRUE(tGeometryInput.mInput.get<input_parser::marvel>().wolverine.has_value());
    EXPECT_EQ(tGeometryInput.mInput.get<input_parser::marvel>().wolverine.value(), 100);

    const auto tAllConstraintsInput = tValidatedInput.get<input_parser::ComponentType::kConstraint>().rawInput();
    ASSERT_EQ(tAllConstraintsInput.size(), 1U);
    const auto& tConstraintInput = tAllConstraintsInput.front().rawInput();
    EXPECT_EQ(tConstraintInput.mComponentType, input_parser::ComponentType::kConstraint);
    EXPECT_EQ(tConstraintInput.mBlockName, "dc");
    ASSERT_TRUE(tConstraintInput.mInput.holds_expected_type<input_parser::dc>());
    const auto& aDCInput = tConstraintInput.mInput.get<input_parser::dc>();
    ASSERT_TRUE(aDCInput.name.has_value());
    EXPECT_EQ(aDCInput.name.value(), "tv-show");
    ASSERT_TRUE(aDCInput.superman.has_value());
    EXPECT_TRUE(aDCInput.superman.value());
    ASSERT_TRUE(aDCInput.batman.has_value());
    EXPECT_EQ(aDCInput.batman.value(), 100U);
}

TEST(ValidatedInput, GetNamedMember)
{
    const auto tCrossLinkedInput = make_test_input(kValidMarvel, kValidDC);
    const auto tValidatedInputOrError = make_validated_input(tCrossLinkedInput);
    const auto& tValidatedInput = tValidatedInputOrError.value();
    const auto tGeometryInput = tValidatedInput.get<input_parser::ComponentType::kGeometry>().rawInput();
    EXPECT_EQ(tGeometryInput.mComponentType, input_parser::ComponentType::kGeometry);
    EXPECT_EQ(tGeometryInput.mBlockName, "marvel");

    const auto tAllConstraintsInput = tValidatedInput.get<input_parser::ComponentType::kConstraint>().rawInput();
    const auto& tConstraintInput = tAllConstraintsInput.front().rawInput();
    EXPECT_EQ(tConstraintInput.mComponentType, input_parser::ComponentType::kConstraint);
    EXPECT_EQ(tConstraintInput.mBlockName, "dc");
}

}  // namespace plato::input_validation::unittest
