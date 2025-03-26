#include <gtest/gtest.h>

#include <any>
#include <functional>

#include "plato/input_parser/BlockStructRule.hpp"
#include "plato/input_parser/ComponentBlockParser.hpp"
#include "plato/input_parser/CrossReference.hpp"
#include "plato/input_parser/GenericBlockRule.hpp"
#include "plato/input_parser/InputBlockStruct.hpp"
#include "plato/utilities/StringUtilities.hpp"

PLATO_INPUT_BLOCK_STRUCT((plato)(input_parser),
                         interface_test_block,
                         (int, arabica, "a caffeinated comment")(double, robusta, "a very caffeinated comment"))

namespace plato::input_parser::unittest
{
namespace
{
namespace bsq = boost::spirit::qi;

}  // namespace

TEST(ComponentBlockParser, Parse)
{
    const auto tInput =
        GenericBlockData{BlockName{"interface_test_block"},
                         {GenericToken{"arabica"}, GenericToken{"42"}, GenericToken{"robusta"}, GenericToken{"-1.0"}}};

    const auto tParser =
        ComponentBlockRule<std::string::const_iterator, interface_test_block, ComponentType::kGeometry>{};
    const auto tParserInterface = ComponentBlockParser{tParser};
    const auto tDataWrapper = tParserInterface.parse(tInput);

    EXPECT_EQ(tDataWrapper.mComponentType, ComponentType::kGeometry);
    EXPECT_EQ(tDataWrapper.mBlockName, "interface_test_block");
    EXPECT_TRUE(tDataWrapper.mInput.has_value());
    ASSERT_TRUE(tDataWrapper.mInput.holds_expected_type<interface_test_block>());
    const auto tResultData = tDataWrapper.mInput.get<interface_test_block>();
    EXPECT_TRUE(tResultData.arabica.has_value());
    EXPECT_EQ(tResultData.arabica.value(), 42);
    EXPECT_TRUE(tResultData.robusta.has_value());
    EXPECT_EQ(tResultData.robusta.value(), -1.0);
}
}  // namespace plato::input_parser::unittest
