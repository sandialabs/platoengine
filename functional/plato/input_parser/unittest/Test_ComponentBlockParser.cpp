#include <gtest/gtest.h>

#include "plato/input_parser/BlockStructRule.hpp"
#include "plato/input_parser/ComponentBlockParser.hpp"
#include "plato/input_parser/GenericBlockRule.hpp"
#include "plato/test_utilities/TestContext.hpp"

PLATO_INPUT_BLOCK_STRUCT((plato)(input_parser),
                         interface_test_block,
                         (int, arabica, "a caffeinated comment")(double, robusta, "a very caffeinated comment"))

namespace plato::input_parser::unittest
{
void check_parsed_data(const InputDataBlock& aParsedData, const test_utilities::TestContext& aTestContext)
{
    EXPECT_EQ(aParsedData.mComponentType, ComponentType::kGeometry) << aTestContext;
    EXPECT_EQ(aParsedData.mBlockName, "interface_test_block") << aTestContext;
    EXPECT_TRUE(aParsedData.mInput.has_value()) << aTestContext;
    ASSERT_TRUE(aParsedData.mInput.holds_expected_type<interface_test_block>()) << aTestContext;

    const auto tResultData = aParsedData.mInput.get<interface_test_block>();
    EXPECT_TRUE(tResultData.arabica.has_value()) << aTestContext;
    EXPECT_EQ(tResultData.arabica.value(), 42) << aTestContext;
    EXPECT_TRUE(tResultData.robusta.has_value()) << aTestContext;
    EXPECT_EQ(tResultData.robusta.value(), 1.0) << aTestContext;
}

TEST(ComponentBlockParser, Parse)
{
    const auto tInput =
        GenericBlockData{BlockName{"interface_test_block"},
                         {GenericToken{"arabica"}, GenericToken{"42"}, GenericToken{"robusta"}, GenericToken{"-1.0"}}};
    {
        const auto tParser =
            ComponentBlockParser{interface_test_block{}, ComponentTypeHelper<ComponentType::kGeometry>{}};
        check_parsed_data(tParser.parse(tInput), TEST_CONTEXT("Direct construction"));
    }
    {
        const auto tParser = make_component_block_parser<interface_test_block, ComponentType::kGeometry>();
        check_parsed_data(tParser.parse(tInput), TEST_CONTEXT("Make function"));
    }
}

}  // namespace plato::input_parser::unittest
