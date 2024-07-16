#define BOOST_PHOENIX_STL_TUPLE_H_  // Work-around for ODR violation in boost phoenix.
                                    // https://github.com/boostorg/phoenix/issues/111

#include <gtest/gtest.h>

#include <string_view>

#include "plato/input_parser/CrossReference.hpp"
#include "plato/input_parser/InputBlockStruct.hpp"
#include "plato/input_parser/InputParser.hpp"
#include "plato/input_parser/Skipper.hpp"

namespace plato::input_parser::unittest
{
/// @brief Type trait for defining a cross reference field. This is not used in any actual input blocks, but is required
/// for defining the CrossReference field.
template <typename T>
struct IsTypeTraitInput
{
    constexpr static bool value = false;
};
}  // namespace plato::input_parser::unittest

// clang-format off
PLATO_INPUT_BLOCK_STRUCT(
    (plato)(input_parser), test, 
    (int, field1)
)

PLATO_INPUT_BLOCK_STRUCT(
    (plato)(input_parser), test_block, 
    (int, field2)
)

PLATO_NAMED_INPUT_BLOCK_STRUCT(
    (plato)(input_parser), test_block_with_name, 
    (int, field3)
)

PLATO_INPUT_BLOCK_STRUCT(
    (plato)(input_parser), block_with_end_field, 
    (bool, field4)
    (double, end_field)
)

/// @brief Test input block that includes a cross reference to another input block with the IsTypeTraitInput type trait.
/// This is used for testing parsing of a Cross-Reference field which requires a type trait template parameter.
/// No input block with IsTypeTraitInput is defined in this test harness. 
PLATO_INPUT_BLOCK_STRUCT(
    (plato)(input_parser), block_with_cross_reference, 
    (int, field)
    (plato::input_parser::CrossReference<plato::input_parser::unittest::IsTypeTraitInput>, hotdog)
)

BOOST_FUSION_DEFINE_STRUCT(
    (plato)(input_parser)(unittest), TestParsedInput,
    (boost::optional<plato::input_parser::test>, mTest)
    (boost::optional<plato::input_parser::test_block>, mTestBlock)
    (std::vector<plato::input_parser::test_block_with_name>, mTestBlockWithName)
    (boost::optional<plato::input_parser::block_with_end_field>, mBlockWithEndField)
    (boost::optional<plato::input_parser::block_with_cross_reference>, mBlockWithCrossReference)
)
// clang-format on

namespace plato::input_parser::unittest
{

template <typename Iterator>
using TestInputParser = NestedStructParser<TestParsedInput, Iterator>;

[[nodiscard]] std::pair<TestParsedInput, bool> parse_input(const std::string_view aInput)
{
    using Iterator = std::string_view::const_iterator;
    TestInputParser<Iterator> tParser;
    TestParsedInput tData;
    auto tIter = aInput.cbegin();
    const auto tSkipper = SkipperRule<Iterator>{};
    const bool tParseResult = phrase_parse(tIter, aInput.cend(), tParser, tSkipper.skipperRule(), tData);
    return {tData, tParseResult};
}

TEST(GenericParser, Test)
{
    const auto tInput = std::string_view{R"(
    begin test
      field1 42
    end
    )"};

    const auto [tParsedData, tWasParsed] = parse_input(tInput);
    EXPECT_TRUE(tWasParsed);
    ASSERT_TRUE(tParsedData.mTest);
    ASSERT_TRUE(tParsedData.mTest->field1);
    EXPECT_EQ(tParsedData.mTest->field1.value(), 42);
}

TEST(GenericParser, TestBlock)
{
    const auto tInput = std::string_view{R"(
    begin test_block
      field2 42
    end
    )"};
    const auto [tParsedData, tWasParsed] = parse_input(tInput);
    EXPECT_TRUE(tWasParsed);
    ASSERT_TRUE(tParsedData.mTestBlock);
    ASSERT_TRUE(tParsedData.mTestBlock->field2);
    EXPECT_EQ(tParsedData.mTestBlock->field2.value(), 42);
}

TEST(GenericParser, TestBlockWithName)
{
    const auto tInput = std::string_view{R"(
    begin test_block_with_name frank
      field3 42
    end
    )"};
    const auto [tParsedData, tWasParsed] = parse_input(tInput);
    EXPECT_TRUE(tWasParsed);
    ASSERT_FALSE(tParsedData.mTestBlockWithName.empty());
    ASSERT_TRUE(tParsedData.mTestBlockWithName.front().name);
    EXPECT_EQ(tParsedData.mTestBlockWithName.front().name.value(), "frank");
    ASSERT_TRUE(tParsedData.mTestBlockWithName.front().field3);
    EXPECT_EQ(tParsedData.mTestBlockWithName.front().field3.value(), 42);
}

TEST(GenericParser, BlockWithEndField)
{
    const auto tInput = std::string_view{R"(
    begin block_with_end_field
      field4 true
      end_field 42.0
    end
    )"};
    const auto [tParsedData, tWasParsed] = parse_input(tInput);
    EXPECT_TRUE(tWasParsed);
    ASSERT_TRUE(tParsedData.mBlockWithEndField);
    ASSERT_TRUE(tParsedData.mBlockWithEndField->end_field);
    EXPECT_EQ(tParsedData.mBlockWithEndField->end_field.value(), 42);
}

TEST(GenericParser, BlockWithCrossReference)
{
    const auto tInput = std::string_view{
        R"(
        begin block_with_cross_reference
            field 4
            hotdog oscar_mayer
        end
    )"};

    const auto [tParsedData, tWasParsed] = parse_input(tInput);
    EXPECT_TRUE(tWasParsed);
    ASSERT_TRUE(tParsedData.mBlockWithCrossReference);
    ASSERT_TRUE(tParsedData.mBlockWithCrossReference->hotdog);
    EXPECT_EQ(tParsedData.mBlockWithCrossReference->hotdog.value().mName, "oscar_mayer");
}

}  // namespace plato::input_parser::unittest