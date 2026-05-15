#include <gtest/gtest.h>

#include "plato/input_parser/BlockStructRule.hpp"
#include "plato/input_parser/ComponentBlockParser.hpp"
#include "plato/input_parser/GenericBlockRule.hpp"
#include "plato/test_utilities/TestContext.hpp"

PLATO_INPUT_BLOCK_STRUCT((plato)(input_parser),
                         interface_test_block,
                         plato::components::ComponentType::kGeometry,
                         (int, arabica, "a caffeinated comment")(double, robusta, "a very caffeinated comment"))

namespace plato::input_parser::unittest
{
void check_parsed_data(const ComponentBlockParser::ParsedDataOrError& aParsedDataOrError,
                       const test_utilities::TestContext& aTestContext)
{
    ASSERT_TRUE(aParsedDataOrError.hasValue());
    const auto& aParsedData = aParsedDataOrError.value();

    EXPECT_EQ(aParsedData.mComponentType, components::ComponentType::kGeometry) << aTestContext;
    EXPECT_EQ(aParsedData.mBlockName, "interface_test_block") << aTestContext;
    EXPECT_TRUE(aParsedData.mInput.hasValue()) << aTestContext;
    ASSERT_TRUE(aParsedData.mInput.holdsExpectedType<interface_test_block>()) << aTestContext;

    const auto tResultData = aParsedData.mInput.get<interface_test_block>();
    EXPECT_TRUE(tResultData.arabica.has_value()) << aTestContext;
    EXPECT_EQ(tResultData.arabica.value(), 42) << aTestContext;
    EXPECT_TRUE(tResultData.robusta.has_value()) << aTestContext;
    EXPECT_EQ(tResultData.robusta.value(), -1.0) << aTestContext;
}

TEST(ComponentBlockParser, Parse)
{
    const auto tInput =
        GenericBlockData{BlockName{"interface_test_block"},
                         {GenericToken{"robusta"}, GenericToken{"-1.0"}, GenericToken{"arabica"}, GenericToken{"42"}}};
    {
        const auto tParser =
            ComponentBlockParser{interface_test_block{}, ComponentTypeHelper<components::ComponentType::kGeometry>{}};
        check_parsed_data(tParser.parse(tInput), TEST_CONTEXT("Direct construction"));
    }
    {
        const auto tParser = make_component_block_parser<interface_test_block, components::ComponentType::kGeometry>();
        check_parsed_data(tParser.parse(tInput), TEST_CONTEXT("Make function"));
    }
}

TEST(ComponentBlockParser, ParseWithError)
{
    const auto tErroneousInput = GenericBlockData{
        BlockName{"interface_test_block"},
        {GenericToken{"robusicta"}, GenericToken{"-1.0"}, GenericToken{"arabica"}, GenericToken{"42"}}};

    const auto tParser =
        ComponentBlockParser{interface_test_block{}, ComponentTypeHelper<components::ComponentType::kGeometry>{}};
    const auto tParsedInputOrError = tParser.parse(tErroneousInput);

    ASSERT_TRUE(tParsedInputOrError.hasError());
    EXPECT_FALSE(tParsedInputOrError.error().empty());
}

TEST(ComponentBlockParser, ComponentType)
{
    {
        const auto tParser =
            ComponentBlockParser{interface_test_block{}, ComponentTypeHelper<components::ComponentType::kGeometry>{}};
        EXPECT_EQ(tParser.componentType(), components::ComponentType::kGeometry);
    }
    {
        const auto tParser =
            ComponentBlockParser{interface_test_block{}, ComponentTypeHelper<components::ComponentType::kFilter>{}};
        EXPECT_EQ(tParser.componentType(), components::ComponentType::kFilter);
    }
    {
        const auto tParser =
            ComponentBlockParser{interface_test_block{}, ComponentTypeHelper<components::ComponentType::kObjective>{}};
        EXPECT_EQ(tParser.componentType(), components::ComponentType::kObjective);
    }
}

}  // namespace plato::input_parser::unittest
