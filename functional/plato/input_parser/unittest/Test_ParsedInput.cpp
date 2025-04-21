#include <gtest/gtest.h>

#include <boost/optional/optional_io.hpp>
#include <unordered_map>

#include "plato/input_parser/BlockStructRule.hpp"
#include "plato/input_parser/ComponentBlockParser.hpp"
#include "plato/input_parser/GenericBlockRule.hpp"
#include "plato/input_parser/InputBlockData.hpp"
#include "plato/input_parser/ParsedInput.hpp"
#include "plato/test_utilities/TestContext.hpp"
#include "plato/utilities/NamedType.hpp"

// clang-format off
PLATO_INPUT_BLOCK_STRUCT((plato)(input_parser),
                         fruits, plato::input_parser::ComponentType::kGeometry,
                         (int, banana, "It's a yellow oblong fruit.")
                         (double, apple, "It's a red or green thing."))

PLATO_NAMED_INPUT_BLOCK_STRUCT((plato)(input_parser),
                         vegetables, plato::input_parser::ComponentType::kObjective,
                         (bool, tomato, "Is it a fruit?")
                         (unsigned int, potato, "Boil 'em, mash 'em, stick 'em in a stew."))

PLATO_INPUT_BLOCK_STRUCT((plato)(input_parser),
                         rocks, plato::input_parser::ComponentType::kFilter,
                         (int, shininess, ""))

// clang-format on

namespace plato::input_parser::unittest
{
namespace
{
const auto kComponentParsers = std::unordered_map<std::string, ComponentBlockParser>{
    {"fruits", ComponentBlockParser{fruits{}, ComponentTypeHelper<ComponentType::kGeometry>{}}},
    {"vegetables", ComponentBlockParser{vegetables{}, ComponentTypeHelper<ComponentType::kObjective>{}}}};

void check_component_type(const std::vector<InputDataBlock>& aInputsForComponent,
                          const ComponentType aComponentType,
                          const test_utilities::TestContext& aTestContext)
{
    for (const auto& aInput : aInputsForComponent)
    {
        EXPECT_EQ(aInput.mComponentType, aComponentType) << aTestContext;
    }
}

void check_block_names(const std::vector<InputDataBlock>& aInputsForComponent,
                       const std::vector<std::string>& aBlockNames,
                       const test_utilities::TestContext& aTestContext)
{
    EXPECT_FALSE(aBlockNames.empty());

    for (const auto& tExpectedBlockName : aBlockNames)
    {
        EXPECT_TRUE(std::any_of(aInputsForComponent.begin(), aInputsForComponent.end(),
                                [&tExpectedBlockName](const auto& aInput)
                                { return aInput.mBlockName == tExpectedBlockName; }))
            << aTestContext;
    }
}

}  // namespace

TEST(ParsedInput, ParsesFullValidInput)
{
    const auto tInput = std::string_view{
        "begin vegetables nightshade\n"
        "  potato 13\n"
        "  tomato true\n"
        "end\n"
        "begin fruits\n"
        "  apple 13.0\n"
        "  banana 42\n"
        "end\n"};

    const auto tParsedInputOrError = make_parsed_input(std::string{tInput}, kComponentParsers);
    ASSERT_TRUE(tParsedInputOrError.hasValue());

    const auto& tParsedInput = tParsedInputOrError.value();
    const auto& tVegetableInputs = tParsedInput.get<ComponentType::kObjective>();
    ASSERT_EQ(tVegetableInputs.size(), 1U);
    EXPECT_EQ(tVegetableInputs.front().mBlockName, "vegetables");
    EXPECT_EQ(tVegetableInputs.front().mComponentType, ComponentType::kObjective);
    ASSERT_TRUE(tVegetableInputs.front().mInput.holdsExpectedType<vegetables>());
    const auto tVegetableInput = tVegetableInputs.front().mInput.get<vegetables>();
    ASSERT_TRUE(tVegetableInput.name.has_value());
    EXPECT_EQ(tVegetableInput.name.value(), "nightshade");
    ASSERT_TRUE(tVegetableInput.potato.has_value());
    EXPECT_EQ(tVegetableInput.potato.value(), 13U);
    ASSERT_TRUE(tVegetableInput.tomato.has_value());
    EXPECT_TRUE(tVegetableInput.tomato.value());

    const auto& tFruitInputs = tParsedInput.get<ComponentType::kGeometry>();
    ASSERT_EQ(tFruitInputs.size(), 1U);
    EXPECT_EQ(tFruitInputs.front().mBlockName, "fruits");
    EXPECT_EQ(tFruitInputs.front().mComponentType, ComponentType::kGeometry);
    ASSERT_TRUE(tFruitInputs.front().mInput.holdsExpectedType<fruits>());
    const auto tFruitInput = tFruitInputs.front().mInput.get<fruits>();
    ASSERT_TRUE(tFruitInput.apple.has_value());
    EXPECT_EQ(tFruitInput.apple.value(), 13.0);
    ASSERT_TRUE(tFruitInput.banana.value());
    EXPECT_EQ(tFruitInput.banana.value(), 42);
}

TEST(ParsedInput, ParsesFullInvalidInput)
{
    using TokenToCheck = utilities::NamedType<std::string_view, struct TokenToCheckTag>;

    const auto tCheckForError = [](const std::string_view aInput, const TokenToCheck aTokenToCheck,
                                   const test_utilities::TestContext& aTestContext)
    {
        const auto tParsedInput = make_parsed_input(std::string{aInput}, kComponentParsers);
        ASSERT_TRUE(tParsedInput.hasError()) << aTestContext;

        const auto& tErrorMessage = tParsedInput.error();
        EXPECT_FALSE(tErrorMessage.empty()) << aTestContext;
        EXPECT_NE(tErrorMessage.find(aTokenToCheck.mValue), std::string::npos) << aTestContext;
    };

    {
        const auto tInput = std::string_view{
            "begin vegetables nightshade\n"
            "  potato 13\n"
            "  tomato true\n"
            "end\n"
            "begn fruits\n"
            "  apple 13.0\n"
            "  banana 42\n"
            "end\n"};
        tCheckForError(tInput, TokenToCheck{std::string_view{"begn"}}, TEST_CONTEXT("Bad begin token"));
    }
    {
        const auto tInput = std::string_view{
            "begin vegetables nightshade\n"
            "  potat 13\n"
            "end\n"};
        tCheckForError(tInput, TokenToCheck{std::string_view{"potat"}}, TEST_CONTEXT("Bad field token"));
    }
    {
        const auto tInput = std::string_view{
            "begin fruit\n"
            "  apple 13.0\n"
            "  banana 42\n"
            "end\n"};
        tCheckForError(tInput, TokenToCheck{std::string_view{"fruit"}}, TEST_CONTEXT("Bad component name"));
    }
}

TEST(ParsedInput, OneComponent)
{
    const auto tGeometry =
        InputDataBlock{/*.mComponentType=*/ComponentType::kGeometry, /*.mBlockName=*/"geo_block", {}};
    const auto tParsedInput = ParsedInput{{tGeometry}};

    const auto& tResult = tParsedInput.get<ComponentType::kGeometry>();
    ASSERT_EQ(tResult.size(), 1U);
    check_component_type(tResult, ComponentType::kGeometry, TEST_CONTEXT("Geometry component type"));
    check_block_names(tResult, {"geo_block"}, TEST_CONTEXT("Geometry block name"));

    EXPECT_TRUE(tParsedInput.get<ComponentType::kConstraint>().empty());
    EXPECT_TRUE(tParsedInput.get<ComponentType::kFilter>().empty());
    EXPECT_TRUE(tParsedInput.get<ComponentType::kObjective>().empty());
    EXPECT_TRUE(tParsedInput.get<ComponentType::kProcessManager>().empty());
}

TEST(ParsedInput, TwoComponents)
{
    const auto tGeometry =
        InputDataBlock{/*.mComponentType=*/ComponentType::kGeometry, /*.mBlockName=*/"geo_block", {}};
    const auto tFilter = InputDataBlock{/*.mComponentType=*/ComponentType::kFilter, /*.mBlockName=*/"filter_block", {}};
    const auto tParsedInput = ParsedInput{{tFilter, tGeometry}};

    const auto& tGeometryResults = tParsedInput.get<ComponentType::kGeometry>();
    ASSERT_EQ(tGeometryResults.size(), 1U);
    check_component_type(tGeometryResults, ComponentType::kGeometry, TEST_CONTEXT("Geometry component type"));
    check_block_names(tGeometryResults, {"geo_block"}, TEST_CONTEXT("Geometry block name"));

    const auto& tFilterResults = tParsedInput.get<ComponentType::kFilter>();
    ASSERT_EQ(tFilterResults.size(), 1U);
    check_component_type(tFilterResults, ComponentType::kFilter, TEST_CONTEXT("Filter component type"));
    check_block_names(tFilterResults, {"filter_block"}, TEST_CONTEXT("Filter block name"));

    EXPECT_TRUE(tParsedInput.get<ComponentType::kConstraint>().empty());
    EXPECT_TRUE(tParsedInput.get<ComponentType::kObjective>().empty());
    EXPECT_TRUE(tParsedInput.get<ComponentType::kProcessManager>().empty());
}

TEST(ParsedInput, ThreeComponentsMultipleEntries)
{
    const auto tConstraint1 =
        InputDataBlock{/*.mComponentType=*/ComponentType::kConstraint, /*.mBlockName=*/"constraint_block_1", {}};
    const auto tConstraint2 =
        InputDataBlock{/*.mComponentType=*/ComponentType::kConstraint, /*.mBlockName=*/"constraint_block_2", {}};
    const auto tObjective1 =
        InputDataBlock{/*.mComponentType=*/ComponentType::kObjective, /*.mBlockName=*/"objective_block_1", {}};
    const auto tObjective2 =
        InputDataBlock{/*.mComponentType=*/ComponentType::kObjective, /*.mBlockName=*/"objective_block_2", {}};
    const auto tProcessManager =
        InputDataBlock{/*.mComponentType=*/ComponentType::kProcessManager, /*.mBlockName=*/"process_manager_block", {}};

    const auto tParsedInput = ParsedInput{{tConstraint1, tProcessManager, tObjective1, tConstraint2, tObjective2}};

    const auto& tObjectiveResults = tParsedInput.get<ComponentType::kObjective>();
    ASSERT_EQ(tObjectiveResults.size(), 2U);
    check_component_type(tObjectiveResults, ComponentType::kObjective, TEST_CONTEXT("Objective component types"));
    check_block_names(tObjectiveResults, {"objective_block_1", "objective_block_2"},
                      TEST_CONTEXT("Objective block names"));

    const auto& tConstraintResults = tParsedInput.get<ComponentType::kConstraint>();
    ASSERT_EQ(tConstraintResults.size(), 2U);
    check_component_type(tConstraintResults, ComponentType::kConstraint, TEST_CONTEXT("Constraint component types"));
    check_block_names(tConstraintResults, {"constraint_block_1", "constraint_block_2"},
                      TEST_CONTEXT("Constraint block names"));

    const auto& tProcessManagerResults = tParsedInput.get<ComponentType::kProcessManager>();
    ASSERT_EQ(tProcessManagerResults.size(), 1U);
    check_component_type(tProcessManagerResults, ComponentType::kProcessManager,
                         TEST_CONTEXT("Process manager component types"));
    check_block_names(tProcessManagerResults, {"process_manager_block"}, TEST_CONTEXT("Process manager block names"));

    EXPECT_TRUE(tParsedInput.get<ComponentType::kGeometry>().empty());
    EXPECT_TRUE(tParsedInput.get<ComponentType::kFilter>().empty());
}

TEST(ParsedInput, GetInputBlockWithType)
{
    const auto tObjectiveData1 =
        input_parser::vegetables{/*.name=*/std::string{"objective 1"}, /*.tomato=*/true, /*.potato=*/10U};
    const auto tObjective1 = InputDataBlock{/*.mComponentType=*/ComponentType::kObjective, /*.mBlockName=*/"vegetables",
                                            InputBlockWrapper{tObjectiveData1}};

    const auto tObjectiveData2 =
        input_parser::vegetables{/*.name=*/std::string{"objective 2"}, /*.tomato=*/false, /*.potato=*/11U};
    const auto tObjective2 = InputDataBlock{/*.mComponentType=*/ComponentType::kObjective, /*.mBlockName=*/"vegetables",
                                            InputBlockWrapper{tObjectiveData2}};

    const auto tGeometryData = input_parser::fruits{/*.banana=*/42, /*.apple=*/13.0};
    const auto tGeometry = InputDataBlock{/*.mComponentType=*/ComponentType::kGeometry, /*.mBlockName=*/"fruits",
                                          InputBlockWrapper{tGeometryData}};

    const auto tParsedInput = ParsedInput{{tObjective1, tGeometry, tObjective2}};

    ASSERT_EQ(tParsedInput.get<input_parser::vegetables>().size(), 2U);
    EXPECT_EQ(tParsedInput.get<input_parser::vegetables>().front().name, tObjectiveData1.name);
    EXPECT_EQ(tParsedInput.get<input_parser::vegetables>().back().name, tObjectiveData2.name);
    ASSERT_EQ(tParsedInput.get<input_parser::fruits>().size(), 1U);
    EXPECT_EQ(tParsedInput.get<input_parser::fruits>().front().apple, tGeometryData.apple);
    EXPECT_TRUE(tParsedInput.get<input_parser::rocks>().empty());
}

}  // namespace plato::input_parser::unittest
