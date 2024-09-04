#include <gtest/gtest.h>

#include "plato/input_parser/UserDefinedTokenList.hpp"
#include "plato/input_parser/unittest/Test_Helpers.hpp"
#include "plato/test_utilities/TestContext.hpp"

namespace plato::input_parser::unittest
{
namespace
{
struct LowerCaseOnly
{
    constexpr const char* operator()() const { return "a-z "; }
};

using ListType = UserDefinedTokenList<LowerCaseOnly>;
}  // namespace

TEST(UserDefinedTokenList, ValidChars)
{
    const auto tToken = UserDefinedTokenList<LowerCaseOnly>{};
    EXPECT_EQ(tToken.kValidChars, LowerCaseOnly{}());
}

TEST(UserDefinedTokenList, Insert)
{
    auto tTokenList = UserDefinedTokenList<LowerCaseOnly>{};
    EXPECT_TRUE(tTokenList.mList.empty());

    // Check insert
    const auto tString1 = std::string{"mario"};
    tTokenList.insert(tTokenList.mList.end(), tString1);
    ASSERT_EQ(tTokenList.mList.size(), 1U);
    EXPECT_EQ(tTokenList.mList.front(), tString1);

    const auto tString2 = std::string{"luigi"};
    tTokenList.insert(tTokenList.mList.end(), tString2);
    ASSERT_EQ(tTokenList.mList.size(), 2U);
    EXPECT_EQ(tTokenList.mList.front(), tString1);
    EXPECT_EQ(tTokenList.mList.back(), tString2);
}

TEST(UserDefinedTokenList, BeginIterator)
{
    const auto tFirstEntry = std::string{"mario"};
    auto tTokenList = UserDefinedTokenList<LowerCaseOnly>{{tFirstEntry, "luigi", "peach"}};
    EXPECT_EQ(*tTokenList.begin(), tFirstEntry);

    const auto tNewEntry = std::string{"toad"};
    *tTokenList.begin() = tNewEntry;
    EXPECT_EQ(*tTokenList.begin(), tNewEntry);
}

TEST(UserDefinedTokenList, EndIterator)
{
    const auto tLastEntry = std::string{"hammerbrother"};
    auto tTokens = std::vector<std::string>{"goomba", "koopa", tLastEntry};
    auto tTokenList = UserDefinedTokenList<LowerCaseOnly>{tTokens};
    EXPECT_EQ(*std::prev(tTokens.end()), tLastEntry);

    const auto tNewEntry = std::string{"shyguy"};
    *std::prev(tTokens.end()) = tNewEntry;
    EXPECT_EQ(*std::prev(tTokens.end()), tNewEntry);
}

TEST(UserDefinedTokenList, BeginConstIterator)
{
    const auto tFirstEntry = std::string{"bowser"};
    const auto tTokenList = UserDefinedTokenList<LowerCaseOnly>{{tFirstEntry, "boo", "yoshi"}};
    EXPECT_EQ(*tTokenList.begin(), tFirstEntry);
}

TEST(UserDefinedTokenList, EndConstIterator)
{
    const auto tLastEntry = std::string{"waluigi"};
    const auto tTokenList = UserDefinedTokenList<LowerCaseOnly>{{"wario", "drybones", tLastEntry}};
    EXPECT_EQ(*std::prev(tTokenList.end()), tLastEntry);
}

TEST(UserDefinedTokenList, StreamInsertion)
{
    const auto tToken1 = std::string{"racoon"};
    const auto tToken2 = std::string{"frog"};
    const auto tTokenList = UserDefinedTokenList<LowerCaseOnly>{{tToken1, tToken2}};

    auto tStream = std::stringstream{};
    tStream << tTokenList;

    const auto tExpected = tToken1 + ", " + tToken2;
    EXPECT_EQ(tStream.str(), tExpected);
}

TEST(UserDefinedTokenList, SuccessfulParseSingleEntries)
{
    const auto tToken1 = std::string{"birdo"};
    const auto tAllTokens = std::vector{tToken1};
    const auto [tResult, tSuccess] = parse_input<ListType>(tToken1);
    EXPECT_TRUE(tSuccess);
    EXPECT_EQ(tResult.mList, std::vector{tToken1});
}

TEST(UserDefinedTokenList, SuccessfulParseMultipleEntries)
{
    const auto tToken1 = std::string{"birdo"};
    const auto tToken2 = std::string{"daisy"};
    const auto tToken3 = std::string{"dry bones"};
    const auto tAllTokens = std::vector{tToken1, tToken2, tToken3};
    const auto tTokenList = utilities::concatenate_container(tAllTokens, ", ");

    const auto [tResult, tSuccess] = parse_input<ListType>(tTokenList);
    EXPECT_TRUE(tSuccess);

    EXPECT_EQ(tResult.mList, tAllTokens);
}

TEST(UserDefinedTokenList, FailedParseBadCharacters)
{
    const auto tBadInput = std::string_view{"dry-bones, koopa-troopa"};
    const auto [tResult, tSuccess] = parse_input<ListType>(tBadInput);
    EXPECT_FALSE(tSuccess);
}

}  // namespace plato::input_parser::unittest
