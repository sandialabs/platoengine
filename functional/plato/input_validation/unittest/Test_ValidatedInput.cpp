#include <gtest/gtest.h>

#include <boost/optional/optional_io.hpp>
#include <fstream>

#include "plato/input_parser/CrossLinkedInput.hpp"
#include "plato/input_parser/InputBlockData.hpp"
#include "plato/input_parser/InputBlockStruct.hpp"
#include "plato/input_parser/ParsedInput.hpp"
#include "plato/input_validation/ValidatedInput.hpp"
#include "plato/input_validation/ValidationRegistration.hpp"
#include "plato/test_utilities/FileCreatingTestFixture.hpp"
#include "plato/test_utilities/TestContext.hpp"

namespace
{
using GeometryCrossReference = plato::input_parser::CrossReference<plato::input_parser::ComponentType::kGeometry>;
}

// Define input structs
// clang-format off
PLATO_INPUT_BLOCK_STRUCT((plato)(input_parser),
                         marvel, input_parser::ComponentType::kGeometry,
                         (double, cyclops, "")
                         (int, wolverine, ""))

PLATO_NAMED_INPUT_BLOCK_STRUCT((plato)(input_parser),
                               dc, plato::input_parser::ComponentType::kConstraint,
                               (bool, superman, "")
                               (unsigned int, batman, "")
                               (GeometryCrossReference, multiverse, ""))
// clang-format on

namespace plato::input_validation::unittest
{
const auto kInputFilePath = std::filesystem::path{"test-input.i"};

namespace
{
constexpr auto kMarvelValidationErrorMessage = std::string_view{"Marvel error!"};
constexpr auto kDCValidationErrorMessage = std::string_view{"DC error!"};
constexpr auto kParsedInputValidationErrorMessage = std::string_view{"Multiverse error!"};

const auto kValidMarvel = input_parser::marvel{/*.cyclops=*/42.0, /*.wolverine=*/100};
const auto kValidDC = input_parser::dc{/*.name=*/std::string{"tv-show"}, /*.superman=*/true, /*.batman=*/100U,
                                       /*.multiverse=*/boost::none};

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

[[nodiscard]] auto validate_parsed_input(const input_parser::ParsedInput& aParsedInput) -> std::optional<std::string>
{
    if (aParsedInput.get<input_parser::ComponentType::kGeometry>().empty())
    {
        return std::string{kParsedInputValidationErrorMessage};
    }
    return std::nullopt;
}

/// @brief Test fixture for registering validation functions
class ValidatedInputRegistrationFixture : virtual public ::testing::Test
{
   public:
    ValidatedInputRegistrationFixture()
    {
        [[maybe_unused]] const auto kBlockValidationRegistration = CrossReferencedInputValidationRegistration<>{
            {[](const input_parser::marvel& aInput) { return validate_wolverine(aInput.wolverine); },
             [](const input_parser::dc& aInput) { return validate_superman(aInput.superman); }}};

        [[maybe_unused]] const auto kInputValidationRegistration = ParsedInputValidationRegistration<>{{
            [](const input_parser::ParsedInput& aInput) { return validate_parsed_input(aInput); },
        }};
    }
    ~ValidatedInputRegistrationFixture()
    {
        detail::registered_validation_functions<input_parser::CrossReferencedInput>().clear();
        detail::registered_validation_functions<input_parser::ParsedInput>().clear();
    }
};

class ValidatedInputFileFixture : public test_utilities::FileCreatingTestFixture,
                                  public ValidatedInputRegistrationFixture
{
   public:
    ValidatedInputFileFixture() : test_utilities::FileCreatingTestFixture{kInputFilePath} {}
};

auto make_test_parsed_input(const std::optional<input_parser::marvel>& aMarvelInput,
                            const std::optional<input_parser::dc>& aDCInput) -> input_parser::ParsedInput
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
    return input_parser::ParsedInput{std::move(tInputs)};
}

auto make_test_input(const std::optional<input_parser::marvel>& aMarvelInput,
                     const std::optional<input_parser::dc>& aDCInput)
{
    auto tCrossLinkedInput = input_parser::make_cross_linked_input(make_test_parsed_input(aMarvelInput, aDCInput));
    EXPECT_TRUE(tCrossLinkedInput.hasValue());
    return tCrossLinkedInput;
}

}  // namespace

TEST_F(ValidatedInputRegistrationFixture, ConstructionValidInput)
{
    {
        const auto tCrossLinkedInput = make_test_input(kValidMarvel, kValidDC);
        ASSERT_TRUE(tCrossLinkedInput.hasValue());
        const auto tValidatedInput = make_validated_input(tCrossLinkedInput.value());
        EXPECT_TRUE(tValidatedInput.hasValue());
    }
    {
        const auto tParsedInput = make_test_parsed_input(kValidMarvel, kValidDC);
        const auto tValidatedInput = make_validated_input(tParsedInput);
        EXPECT_TRUE(tValidatedInput.hasValue());
    }
}

TEST_F(ValidatedInputRegistrationFixture, ConstructionOneEntryInvalidInput)
{
    const auto kInvalidMarvel = input_parser::marvel{/*.cyclops=*/42.0, /*.wolverine=*/101};

    const auto tCheckErrors = [](const auto& aValidatedInputOrError, const test_utilities::TestContext& aTestContext)
    {
        ASSERT_TRUE(aValidatedInputOrError.hasError()) << aTestContext;
        EXPECT_EQ(aValidatedInputOrError.error(), kMarvelValidationErrorMessage) << aTestContext;
    };
    {
        const auto tCrossLinkedInput = make_test_input(kInvalidMarvel, std::nullopt);
        ASSERT_TRUE(tCrossLinkedInput.hasValue());
        const auto tValidatedInput = make_validated_input(tCrossLinkedInput.value());
        tCheckErrors(tValidatedInput, TEST_CONTEXT("Using make_test_input"));
    }
    {
        const auto tParsedInput = make_test_parsed_input(kInvalidMarvel, std::nullopt);
        const auto tValidatedInput = make_validated_input(tParsedInput);
        tCheckErrors(tValidatedInput, TEST_CONTEXT("Using make_test_parsed_input"));
    }
}

TEST_F(ValidatedInputRegistrationFixture, ConstructionTwoEntriesInvalidInput)
{
    const auto kInvalidMarvel = input_parser::marvel{/*.cyclops=*/42.0, /*.wolverine=*/101};
    const auto kInvalidDC = input_parser::dc{/*.name=*/std::string{"tv-show"}, /*.superman=*/boost::none,
                                             /*.batman=*/100U, /*.multiverse=*/GeometryCrossReference{}};
    const auto tCrossLinkedInput = make_test_input(kInvalidMarvel, kInvalidDC);
    ASSERT_TRUE(tCrossLinkedInput.hasValue());
    const auto tValidatedInput = make_validated_input(tCrossLinkedInput.value());

    ASSERT_TRUE(tValidatedInput.hasError());
    EXPECT_EQ(tValidatedInput.error(),
              std::string{kDCValidationErrorMessage} + "\n" + std::string{kMarvelValidationErrorMessage});
}

TEST_F(ValidatedInputRegistrationFixture, ConstructionInvalidParsedInput)
{
    // No parsers are registered, so no cross-link errors should occur
    const auto tCrossLinkedInput = make_test_input(std::nullopt, kValidDC);
    ASSERT_TRUE(tCrossLinkedInput.hasValue());
    const auto tValidatedInput = make_validated_input(tCrossLinkedInput.value());

    ASSERT_TRUE(tValidatedInput.hasError());
    EXPECT_EQ(tValidatedInput.error(), std::string{kParsedInputValidationErrorMessage});
}

TEST_F(ValidatedInputRegistrationFixture, ConstructionInvalidCrossLinkedInput)
{
    [[maybe_unused]] const auto kLandCreaturesParserRegistration =
        input_parser::ComponentParserRegistration<input_parser::marvel>{};
    [[maybe_unused]] const auto kSeaCreaturesParserRegistration =
        input_parser::ComponentParserRegistration<input_parser::dc>{};

    const auto tParsedInput = make_test_parsed_input(std::nullopt, kValidDC);
    const auto tValidatedInput = make_validated_input(tParsedInput);

    ASSERT_TRUE(tValidatedInput.hasError());
    const auto tExpectedError = std::string{
        "Error satisfying cross-reference for dc: No components are defined that match the cross-reference's required "
        "component type"};
    EXPECT_EQ(tValidatedInput.error(), tExpectedError);

    input_parser::registered_component_parsers().clear();
    input_parser::registered_cross_linkers().clear();
}

TEST_F(ValidatedInputRegistrationFixture, GetMember)
{
    const auto tCrossLinkedInput = make_test_input(kValidMarvel, kValidDC);
    ASSERT_TRUE(tCrossLinkedInput.hasValue());
    const auto tValidatedInputOrError = make_validated_input(tCrossLinkedInput.value());

    ASSERT_TRUE(tValidatedInputOrError.hasValue());
    const auto& tValidatedInput = tValidatedInputOrError.value();

    const auto tGeometryInput = tValidatedInput.get<input_parser::ComponentType::kGeometry>().rawInput();
    EXPECT_EQ(tGeometryInput.mComponentType, input_parser::ComponentType::kGeometry);
    EXPECT_EQ(tGeometryInput.mBlockName, "marvel");
    ASSERT_TRUE(tGeometryInput.mInput.holdsExpectedType<input_parser::marvel>());
    ASSERT_TRUE(tGeometryInput.mInput.get<input_parser::marvel>().cyclops.has_value());
    EXPECT_EQ(tGeometryInput.mInput.get<input_parser::marvel>().cyclops.value(), 42.0);
    ASSERT_TRUE(tGeometryInput.mInput.get<input_parser::marvel>().wolverine.has_value());
    EXPECT_EQ(tGeometryInput.mInput.get<input_parser::marvel>().wolverine.value(), 100);

    const auto tAllConstraintsInput = tValidatedInput.get<input_parser::ComponentType::kConstraint>().rawInput();
    ASSERT_EQ(tAllConstraintsInput.size(), 1U);
    const auto& tConstraintInput = tAllConstraintsInput.front().rawInput();
    EXPECT_EQ(tConstraintInput.mComponentType, input_parser::ComponentType::kConstraint);
    EXPECT_EQ(tConstraintInput.mBlockName, "dc");
    ASSERT_TRUE(tConstraintInput.mInput.holdsExpectedType<input_parser::dc>());
    const auto& aDCInput = tConstraintInput.mInput.get<input_parser::dc>();
    ASSERT_TRUE(aDCInput.name.has_value());
    EXPECT_EQ(aDCInput.name.value(), "tv-show");
    ASSERT_TRUE(aDCInput.superman.has_value());
    EXPECT_TRUE(aDCInput.superman.value());
    ASSERT_TRUE(aDCInput.batman.has_value());
    EXPECT_EQ(aDCInput.batman.value(), 100U);
}

TEST_F(ValidatedInputRegistrationFixture, GetNamedMember)
{
    const auto tCrossLinkedInput = make_test_input(kValidMarvel, kValidDC);
    ASSERT_TRUE(tCrossLinkedInput.hasValue());
    const auto tValidatedInputOrError = make_validated_input(tCrossLinkedInput.value());
    const auto& tValidatedInput = tValidatedInputOrError.value();
    const auto tGeometryInput = tValidatedInput.get<input_parser::ComponentType::kGeometry>().rawInput();
    EXPECT_EQ(tGeometryInput.mComponentType, input_parser::ComponentType::kGeometry);
    EXPECT_EQ(tGeometryInput.mBlockName, "marvel");

    const auto tAllConstraintsInput = tValidatedInput.get<input_parser::ComponentType::kConstraint>().rawInput();
    const auto& tConstraintInput = tAllConstraintsInput.front().rawInput();
    EXPECT_EQ(tConstraintInput.mComponentType, input_parser::ComponentType::kConstraint);
    EXPECT_EQ(tConstraintInput.mBlockName, "dc");
}

TEST_F(ValidatedInputRegistrationFixture, GetInputBlock)
{
    const auto tCrossLinkedInput = make_test_input(kValidMarvel, kValidDC);
    ASSERT_TRUE(tCrossLinkedInput.hasValue());
    const auto tValidatedInputOrError = make_validated_input(tCrossLinkedInput.value());

    ASSERT_TRUE(tValidatedInputOrError.hasValue());
    const auto& tValidatedInput = tValidatedInputOrError.value();
    const auto tValidatedMarvelInput = tValidatedInput.get<input_parser::ComponentType::kGeometry>();
    const auto tMarvelInput = get_input_block<input_parser::marvel>(tValidatedMarvelInput);

    EXPECT_EQ(tMarvelInput.cyclops, 42.0);
    EXPECT_EQ(tMarvelInput.wolverine, 100);
}

TEST_F(ValidatedInputFileFixture, ParseAndValidate)
{
    [[maybe_unused]] const auto tMarvelParserRegistration =
        input_parser::ComponentParserRegistration<input_parser::marvel>{};
    [[maybe_unused]] const auto tDCParserRegistration = input_parser::ComponentParserRegistration<input_parser::dc>{};

    const auto tCheckParsedInput =
        [](const auto& aValidatedInputOrError, const test_utilities::TestContext& aTestContext)
    {
        ASSERT_TRUE(aValidatedInputOrError.hasValue()) << aValidatedInputOrError.error();
        const auto& tValidatedInput = aValidatedInputOrError.value();
        // Marvel
        const auto& tValidatedGeometryInput = tValidatedInput.template get<input_parser::ComponentType::kGeometry>();
        const auto& tRawGeometry = get_input_block<input_parser::marvel>(tValidatedGeometryInput);
        EXPECT_EQ(tRawGeometry.cyclops, 13.0) << aTestContext;
        EXPECT_EQ(tRawGeometry.wolverine, 100) << aTestContext;
        // DC
        const auto& tValidatedDCInput = tValidatedInput.template get<input_parser::ComponentType::kConstraint>();
        ASSERT_EQ(tValidatedDCInput.rawInput().size(), 1U) << aTestContext;
        const auto& tRawConstraint = get_input_block<input_parser::dc>(tValidatedDCInput.rawInput().front());
        EXPECT_EQ(tRawConstraint.name, std::string{"epic"}) << aTestContext;
        EXPECT_EQ(tRawConstraint.superman, true) << aTestContext;
        EXPECT_EQ(tRawConstraint.batman, 11U) << aTestContext;
    };

    constexpr auto tInputText =
        "begin marvel\n"
        "  cyclops 13.0\n"
        "  wolverine 100\n"
        "end\n"
        "begin dc epic\n"
        "  superman true\n"
        "  batman 11\n"
        "end";

    {
        const auto tValidatedInputOrError = parse_and_validate_string(tInputText);
        tCheckParsedInput(tValidatedInputOrError, TEST_CONTEXT("Parsed from string"));
    }
    {
        {
            auto tFileStream = std::ofstream{filePath()};
            tFileStream << tInputText;
        }
        const auto tValidatedInputOrError = parse_and_validate_file(filePath());
        tCheckParsedInput(tValidatedInputOrError, TEST_CONTEXT("Parsed from file"));
    }

    input_parser::registered_component_parsers().clear();
    input_parser::registered_cross_linkers().clear();
}

}  // namespace plato::input_validation::unittest
