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

auto parse_generic_block(const std::string& aInput)
{
    auto tIter = aInput.begin();
    const auto tParser = GenericBlockParser<std::string::const_iterator>{};
    auto tData = GenericBlockData{};
    const auto tSkipper = SkipperRule<std::string::const_iterator>{};
    const auto tParseResult = phrase_parse(tIter, aInput.cend(), tParser, tSkipper.skipperRule(), tData);
    return std::make_tuple(tParseResult, tIter, tData);
}
}  // namespace

TEST(GenericBlockToken, ParsesToken)
{
    const auto tInput = std::string_view{"atok3n-with_special_,chars*and(the)!w0rd=end@"};
    auto tIter = tInput.begin();
    auto tData = GenericToken{};
    const auto tSkipper = SkipperRule<std::string_view::const_iterator>{};
    const auto tParseResult =
        phrase_parse(tIter, tInput.cend(), boost::spirit::qi::auto_, tSkipper.skipperRule(), tData);
    EXPECT_TRUE(tParseResult);
    EXPECT_EQ(tInput, tData.mToken);
    EXPECT_EQ(tIter, tInput.cend());
}

TEST(GenericBlockToken, DoesNotParseEnd)
{
    const auto tInput = std::string_view{"end"};
    auto tIter = tInput.begin();
    auto tData = GenericToken{};
    const auto tSkipper = SkipperRule<std::string_view::const_iterator>{};
    const auto tParseResult =
        phrase_parse(tIter, tInput.cend(), boost::spirit::qi::auto_, tSkipper.skipperRule(), tData);
    EXPECT_FALSE(tParseResult);
    EXPECT_NE(tIter, tInput.cend());
    const auto tUnparsedText = std::string(tIter, tInput.cend());
    EXPECT_EQ(tUnparsedText, tInput);
}

TEST(GenericBlockRule, ParsesValidInput)
{
    const auto tCheckForValidInput = [](const std::string& aInput, const test_utilities::TestContext& aTestContext)
    {
        const auto [tParseResult, tResultIterator, tParsedData] = parse_generic_block(aInput);

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
        const auto [tParseResult, tResultIterator, tParsedData] = parse_generic_block(aInput);
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

}  // namespace plato::input_parser::unittest
