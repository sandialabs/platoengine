#include <gtest/gtest.h>

#include "plato/input_parser/ComponentBlockParser.hpp"
#include "plato/input_parser/InputBlockStruct.hpp"
#include "plato/input_parser/ParsedInput.hpp"
#include "plato/input_validation/ValidatedInput.hpp"
#include "plato/input_validation/ValidationRegistration.hpp"

// Define input structs
// clang-format off
PLATO_INPUT_BLOCK_STRUCT((plato)(input_parser),
                         marvel,
                         (double, cyclops, "")
                         (int, wolverine, ""))

PLATO_NAMED_INPUT_BLOCK_STRUCT((plato)(input_parser),
                         dc,
                         (bool, superman, "")
                         (unsigned int, batman, ""))
// clang-format on

namespace plato::input_validation::unittest
{
namespace
{
constexpr auto kMarvelValidationErrorMessage = std::string_view{"Marvel error!"};
constexpr auto kDCValidationErrorMessage = std::string_view{"DC error!"};
constexpr auto kParsedInputValidationErrorMessage = std::string_view{"Bigtime error!"};

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

}  // namespace

TEST(ValidatedInput, ConstructionValidInput)
{
    const auto tMarvelInput = input_parser::InputDataBlock{
        /*.mComponentType=*/input_parser::ComponentType::kGeometry, /*.mBlockName=*/"marvel",
        /*.mInput=*/input_parser::CrossReferencedInput{kValidMarvel}};
    const auto tDCInput =
        input_parser::InputDataBlock{/*.mComponentType=*/input_parser::ComponentType::kConstraint, /*.mBlockName=*/"dc",
                                     /*.mInput=*/input_parser::CrossReferencedInput{kValidDC}};

    const auto tParsedInput = input_parser::NewParsedInput{{tMarvelInput, tDCInput}};

    const auto tValidatedInput = make_validated_input(tParsedInput);

    ASSERT_TRUE(tValidatedInput.hasValue());
}

TEST(ValidatedInput, ConstructionOneEntryInvalidInput)
{
    const auto kInvalidMarvel = input_parser::marvel{/*.cyclops=*/42.0, /*.wolverine=*/101};
    const auto tMarvelInput = input_parser::InputDataBlock{
        /*.mComponentType=*/input_parser::ComponentType::kGeometry, /*.mBlockName=*/"marvel",
        /*.mInput=*/input_parser::CrossReferencedInput{kInvalidMarvel}};
    const auto tDCInput =
        input_parser::InputDataBlock{/*.mComponentType=*/input_parser::ComponentType::kConstraint, /*.mBlockName=*/"dc",
                                     /*.mInput=*/input_parser::CrossReferencedInput{kValidDC}};

    const auto tParsedInput = input_parser::NewParsedInput{{tMarvelInput, tDCInput}};

    const auto tValidatedInput = make_validated_input(tParsedInput);

    ASSERT_TRUE(tValidatedInput.hasError());
    EXPECT_EQ(tValidatedInput.error(), kMarvelValidationErrorMessage);
}

TEST(ValidatedInput, ConstructionTwoEntriesInvalidInput)
{
    const auto kInvalidMarvel = input_parser::marvel{/*.cyclops=*/42.0, /*.wolverine=*/101};
    const auto tMarvelInput = input_parser::InputDataBlock{
        /*.mComponentType=*/input_parser::ComponentType::kGeometry, /*.mBlockName=*/"marvel",
        /*.mInput=*/input_parser::CrossReferencedInput{kInvalidMarvel}};
    const auto kInvalidDC =
        input_parser::dc{/*.name=*/std::string{"tv-show"}, /*.superman=*/boost::none, /*.batman=*/100U};
    const auto tDCInput =
        input_parser::InputDataBlock{/*.mComponentType=*/input_parser::ComponentType::kConstraint, /*.mBlockName=*/"dc",
                                     /*.mInput=*/input_parser::CrossReferencedInput{kInvalidDC}};

    const auto tParsedInput = input_parser::NewParsedInput{{tMarvelInput, tDCInput}};

    const auto tValidatedInput = make_validated_input(tParsedInput);

    ASSERT_TRUE(tValidatedInput.hasError());
    EXPECT_EQ(tValidatedInput.error(),
              std::string{kMarvelValidationErrorMessage} + "\n" + std::string{kDCValidationErrorMessage});
}

TEST(ValidatedInput, ConstructionInvalidParsedInput)
{
    const auto tDCInput =
        input_parser::InputDataBlock{/*.mComponentType=*/input_parser::ComponentType::kConstraint, /*.mBlockName=*/"dc",
                                     /*.mInput=*/input_parser::CrossReferencedInput{kValidDC}};
    const auto tParsedInput = input_parser::NewParsedInput{{tDCInput}};

    const auto tValidatedInput = make_validated_input(tParsedInput);

    ASSERT_TRUE(tValidatedInput.hasError());
    EXPECT_EQ(tValidatedInput.error(), std::string{kParsedInputValidationErrorMessage});
}

}  // namespace plato::input_validation::unittest
