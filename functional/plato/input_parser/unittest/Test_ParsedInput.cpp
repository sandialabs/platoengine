#include <gtest/gtest.h>

#include <unordered_map>

#include "plato/input_parser/BlockStructRule.hpp"
#include "plato/input_parser/ComponentBlockParser.hpp"
#include "plato/input_parser/GenericBlockRule.hpp"

// clang-format off
PLATO_INPUT_BLOCK_STRUCT((plato)(input_parser),
                         fruits,
                         (int, banana, "It's a yellow oblong fruit.")
                         (double, apple, "It's a red or green thing."))

PLATO_INPUT_BLOCK_STRUCT((plato)(input_parser),
                         vegetables,
                         (bool, tomato, "Is it a fruit?")
                         (unsigned int, potato, "Boil 'em, mash 'em, stick 'em in a stew."))
// clang-format on

namespace plato::input_parser::unittest
{
namespace
{
const auto kComponentParsers = std::unordered_map<std::string, ComponentBlockParser>{
    {"fruits", ComponentBlockParser{fruits{}, ComponentTypeHelper<ComponentType::kGeometry>{}}},
    {"vegetables", ComponentBlockParser{vegetables{}, ComponentTypeHelper<ComponentType::kFilter>{}}}};
}

TEST(ParsedInput, ParsesFullValidInput)
{
    const auto tInput = std::string_view{
        "begin vegetables\n"
        "  potato 13\n"
        "  tomato true\n"
        "end\n"
        "begin fruits\n"
        "  apple 13.0\n"
        "  banana 42\n"
        "end\n"};
    const auto tGenericBlocks = parse_generic_blocks(tInput);
    auto tParsedInput = std::vector<InputDataBlock>{};
    std::transform(tGenericBlocks.begin(), tGenericBlocks.end(), std::back_inserter(tParsedInput),
                   [](const auto& aGenericBlock)
                   { return kComponentParsers.at(aGenericBlock.mName.mToken).parse(aGenericBlock); });

    ASSERT_EQ(tParsedInput.size(), 2U);
    EXPECT_EQ(tParsedInput.front().mBlockName, "vegetables");
    EXPECT_EQ(tParsedInput.front().mComponentType, ComponentType::kFilter);
    ASSERT_TRUE(tParsedInput.front().mInput.holds_expected_type<vegetables>());
    const auto tVegetableInput = tParsedInput.front().mInput.get<vegetables>();
    ASSERT_TRUE(tVegetableInput.potato.has_value());
    EXPECT_EQ(tVegetableInput.potato.value(), 13U);
    ASSERT_TRUE(tVegetableInput.tomato.has_value());
    EXPECT_TRUE(tVegetableInput.tomato.value());

    EXPECT_EQ(tParsedInput.back().mBlockName, "fruits");
    EXPECT_EQ(tParsedInput.back().mComponentType, ComponentType::kGeometry);
    ASSERT_TRUE(tParsedInput.back().mInput.holds_expected_type<fruits>());
    const auto tFruitInput = tParsedInput.back().mInput.get<fruits>();
    ASSERT_TRUE(tFruitInput.apple.has_value());
    EXPECT_EQ(tFruitInput.apple.value(), 13.0);
    ASSERT_TRUE(tFruitInput.banana.value());
    EXPECT_EQ(tFruitInput.banana.value(), 42);
}
}  // namespace plato::input_parser::unittest
