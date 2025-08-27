#include <gtest/gtest.h>

#include <any>

#include "plato/input_parser/BlockStructRule.hpp"
#include "plato/input_parser/InputBlockStruct.hpp"

PLATO_INPUT_BLOCK_STRUCT((plato)(input_parser),
                         test_block,
                         plato::components::ComponentType::kGeometry,
                         (bool, field1, "a helpful comment")(int, field2, "a very helpful comment"))

namespace plato::input_parser::unittest
{
TEST(ComponentBlockRule, ParsesToAnyWrapper)
{
    const auto tInput = std::string{
        "field1 true\n"
        "field2 42\n"};
    const auto tParser =
        ComponentBlockRule<std::string::const_iterator, test_block, components::ComponentType::kGeometry>{};

    auto tIter = tInput.begin();
    auto tData = std::any{};
    const auto tSkipper = SkipperRule<std::string::const_iterator>{};
    const auto tParseResult = phrase_parse(tIter, tInput.cend(), tParser.mBlockRule, tSkipper.skipperRule(), tData);

    EXPECT_TRUE(tParseResult);
    ASSERT_TRUE(tData.has_value());
    ASSERT_EQ(tData.type(), typeid(test_block));
    const auto& tCastResult = std::any_cast<test_block>(tData);
    EXPECT_TRUE(tCastResult.field1.has_value());
    EXPECT_TRUE(tCastResult.field1.value());
    EXPECT_TRUE(tCastResult.field2.has_value());
    EXPECT_EQ(tCastResult.field2.value(), 42);
}

}  // namespace plato::input_parser::unittest
