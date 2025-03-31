#include <gtest/gtest.h>

#include <boost/fusion/adapted/std_pair.hpp>
#include <boost/fusion/include/std_pair.hpp>
#include <string_view>

#include "plato/input_parser/GenericBlockRule.hpp"
#include "plato/test_utilities/TestContext.hpp"
#include "plato/utilities/StringUtilities.hpp"
#include "plato/utilities/Zip.hpp"

namespace plato::input_parser::unittest
{
namespace
{
constexpr auto kBeginToken = std::string_view{"begin"};
constexpr auto kEndToken = std::string_view{"end"};
constexpr auto kArbitraryBlockName = std::string_view{"arbitrary_block"};
const auto kArbitraryInput = std::vector{std::string{"lorend"}, std::string{"ipsum"}};

template <typename ParsedType, typename Parser>
auto parse_generic_block(const std::string& aInput, const Parser& aParser)
{
    auto tIter = aInput.begin();
    auto tData = ParsedType{};
    const auto tSkipper = SkipperRule<std::string::const_iterator>{};
    const auto tParseResult = phrase_parse(tIter, aInput.cend(), aParser, tSkipper.skipperRule(), tData);
    return std::make_tuple(tParseResult, tIter, tData);
}
}  // namespace

TEST(GenericBlockToken, ParsesTokens)
{
    const auto tInput = std::string{"atok3n-with_special_,chars*and(the)!w0rd=end@"};
    const auto [tParseResult, tResultIterator, tParsedData] =
        parse_generic_block<GenericToken>(tInput, boost::spirit::qi::auto_);

    EXPECT_TRUE(tParseResult);
    EXPECT_EQ(tInput, tParsedData.mToken);
    EXPECT_EQ(tResultIterator, tInput.cend());
}

TEST(GenericBlockToken, DoesNotParseEnd)
{
    const auto tInput = std::string{"end"};
    const auto [tParseResult, tResultIterator, tParsedData] =
        parse_generic_block<GenericToken>(tInput, boost::spirit::qi::auto_);

    EXPECT_FALSE(tParseResult);
    EXPECT_NE(tResultIterator, tInput.cend());
    const auto tUnparsedText = std::string(tResultIterator, tInput.cend());
    EXPECT_EQ(tUnparsedText, tInput);
}

TEST(GenericBlockRule, ParsesValidInput)
{
    const auto tCheckForValidInput = [](const std::string& aInput, const test_utilities::TestContext& aTestContext)
    {
        const auto tParser = GenericBlockParser<std::string::const_iterator>{};
        const auto [tParseResult, tResultIterator, tParsedData] =
            parse_generic_block<GenericBlockData>(aInput, tParser);

        EXPECT_TRUE(tParseResult) << aTestContext;
        EXPECT_EQ(tResultIterator, aInput.cend()) << "Unparsed text: " << std::string{tResultIterator, aInput.cend()};
        EXPECT_EQ(tParsedData.mName.mToken, kArbitraryBlockName) << aTestContext;
        EXPECT_EQ(tParsedData.mInput.size(), kArbitraryInput.size()) << aTestContext;
        for (const auto& [tParsed, tExpected] : utilities::Zip{tParsedData.mInput, kArbitraryInput})
        {
            EXPECT_EQ(tParsed.mToken, tExpected) << aTestContext;
        }
    };

    {
        const auto tInput = utilities::concatenate(kBeginToken, '\n', kArbitraryBlockName, '\n', kArbitraryInput.at(0),
                                                   ' ', kArbitraryInput.at(1), ' ', kEndToken);
        tCheckForValidInput(tInput, TEST_CONTEXT("Basic input"));
    }
    {
        const auto tInput = std::string{
            "begin arbitrary_block # this is a comment\n"
            " # a comment with an end token\n"
            " lorend # ending with a comment\n"
            "\tipsum\n"
            "end"};

        tCheckForValidInput(tInput, TEST_CONTEXT("Input with comments"));
    }
}

TEST(GenericBlockRule, ErrorsOnBadInput)
{
    const auto tCheckForBadInput = [](const std::string& aInput, const test_utilities::TestContext& aTestContext)
    {
        const auto tParser = GenericBlockParser<std::string::const_iterator>{};
        const auto [tParseResult, tResultIterator, tParsedData] =
            parse_generic_block<GenericBlockData>(aInput, tParser);
        EXPECT_FALSE(tParseResult) << aTestContext;
        EXPECT_NE(tResultIterator, aInput.cend()) << aTestContext;
    };
    {
        const auto tInput = utilities::concatenate(kBeginToken, '\n', kArbitraryBlockName, '\n', kEndToken);
        tCheckForBadInput(tInput, TEST_CONTEXT("Bad content within block"));
    }
    {
        const auto tMisspelledBegin = std::string_view{"begn"};
        const auto tInput = utilities::concatenate(tMisspelledBegin, '\n', kArbitraryBlockName, '\n',
                                                   kArbitraryInput.at(0), '\n', kArbitraryInput.at(1), '\n', kEndToken);
        tCheckForBadInput(tInput, TEST_CONTEXT("Bad begin token"));
    }
    {
        const auto tMisspelledEnd = std::string_view{"ed"};
        const auto tInput = utilities::concatenate(kBeginToken, '\n', kArbitraryBlockName, '\n', kArbitraryInput.at(0),
                                                   '\n', tMisspelledEnd);

        tCheckForBadInput(tInput, TEST_CONTEXT("Bad end token"));
    }
}

TEST(GenericBlockRule, MultipleBlocks)
{
    const auto tInput = std::string{
        "begin arbitrary_block \n"
        " inputs schminputs\n"
        "end\n"
        "begin arbitrary_block_two\n"
        " more inputs!\n"
        "end"};

    const auto tParsedDataOrError = parse_generic_blocks(tInput);

    ASSERT_TRUE(tParsedDataOrError.hasValue());
    const auto& tParsedData = tParsedDataOrError.value();
    ASSERT_EQ(tParsedData.size(), 2U);

    const auto tCheckBlockData = [](const GenericBlockData& aData, const std::string& aExpectedName,
                                    const std::vector<std::string>& aExpectedInputs,
                                    const test_utilities::TestContext& aTestContext)
    {
        EXPECT_EQ(aData.mName.mToken, aExpectedName) << aTestContext;
        for (const auto& [tParsed, tExpected] : utilities::Zip{aData.mInput, aExpectedInputs})
        {
            EXPECT_EQ(tParsed.mToken, tExpected) << aTestContext;
        }
    };

    tCheckBlockData(tParsedData.front(), "arbitrary_block", {"inputs", "schminputs"}, TEST_CONTEXT("Block 1"));
    tCheckBlockData(tParsedData.back(), "arbitrary_block_two", {"more", "inputs!"}, TEST_CONTEXT("Block 2"));
}

TEST(GenericBlockRule, ParseError)
{
    const auto tInput = std::string{
        "begin arbitrary_block \n"
        " inputs schminputs\n"
        "end\n"
        "bgin arbitrary_block_two"
        " more inputs! "
        "end"};

    const auto tParsedDataOrError = parse_generic_blocks(tInput);
    ASSERT_TRUE(tParsedDataOrError.hasError());
    EXPECT_FALSE(tParsedDataOrError.error().empty());
}

TEST(GenericBlockRule, ToString)
{
    const auto tInputText = std::vector<std::string>{"gorilla", "orangutan", "chimpanzee", "gibbon"};
    const auto tGenericBlock = GenericBlockData{BlockName{"arbitrary_block"},
                                                {GenericToken{tInputText.at(0)}, GenericToken{tInputText.at(1)},
                                                 GenericToken{tInputText.at(2)}, GenericToken{tInputText.at(3)}}};
    const auto tResultString = to_string(tGenericBlock);
    const auto tExpectedString = utilities::concatenate_container(tInputText, " ") + " ";
    EXPECT_EQ(tResultString, tExpectedString);
}

}  // namespace plato::input_parser::unittest
