#include <gtest/gtest.h>

#include "plato/input_parser/AutoList.hpp"
#include "plato/input_parser/Bounds.hpp"
#include "plato/input_parser/InputFieldTypes.hpp"
#include "plato/input_parser/UserDefinedToken.hpp"
#include "plato/input_parser/test_utilities/TestHelpers.hpp"

namespace plato::input_parser::unittest
{
namespace
{

using DoubleListType = AutoList<double>;
using StringListType = AutoList<IdentifierString>;
using BoundsListType = AutoList<Bounds>;
using IntListType = AutoList<int>;

struct LowerCaseOnly
{
    constexpr const char* operator()() const { return "a-z"; }
};

using UserDefinedTokenListType = AutoList<UserDefinedToken<LowerCaseOnly>>;
}  // namespace

TEST(AutoList, Insert)
{
    auto tTokenList = DoubleListType{};
    EXPECT_TRUE(tTokenList.mList.empty());

    // Check insert
    const auto tValue1 = 1.0;
    tTokenList.insert(tTokenList.mList.end(), tValue1);
    ASSERT_EQ(tTokenList.mList.size(), 1U);
    EXPECT_EQ(tTokenList.mList.front(), tValue1);

    const auto tValue2 = 2.0;
    tTokenList.insert(tTokenList.mList.end(), tValue2);
    ASSERT_EQ(tTokenList.mList.size(), 2U);
    EXPECT_EQ(tTokenList.mList.front(), tValue1);
    EXPECT_EQ(tTokenList.mList.back(), tValue2);
}

TEST(AutoList, BeginIterator)
{
    const auto tFirstEntry = std::string_view{"mario"};
    auto tTokenList = StringListType{
        {IdentifierString{std::string{tFirstEntry}}, IdentifierString{"luigi"}, IdentifierString{"peach"}}};
    EXPECT_EQ(tTokenList.begin()->mToken, tFirstEntry);

    const auto tNewEntry = IdentifierString{"toad"};
    *tTokenList.begin() = tNewEntry;
    EXPECT_EQ(tTokenList.begin()->mToken, tNewEntry.mToken);
}

TEST(AutoList, EndIterator)
{
    const auto tLastEntry = Bounds{0, 1};
    auto tTokens = std::vector<Bounds>{Bounds{1, 2}, Bounds{2, 3}, tLastEntry};
    auto tTokenList = BoundsListType{tTokens};
    EXPECT_EQ(std::prev(tTokens.end())->mLower, tLastEntry.mLower);
    EXPECT_EQ(std::prev(tTokens.end())->mUpper, tLastEntry.mUpper);

    const auto tNewEntry = Bounds{-1, 1};
    *std::prev(tTokens.end()) = tNewEntry;
    EXPECT_EQ(std::prev(tTokens.end())->mLower, tNewEntry.mLower);
    EXPECT_EQ(std::prev(tTokens.end())->mUpper, tNewEntry.mUpper);
}

TEST(AutoList, BeginConstIterator)
{
    const auto tFirstEntry = 1;
    const auto tTokenList = IntListType{{tFirstEntry, 2, 3}};
    EXPECT_EQ(*tTokenList.begin(), tFirstEntry);
}

TEST(AutoList, EndConstIterator)
{
    const auto tLastEntry = 3;
    const auto tTokenList = IntListType{{5, 4, tLastEntry}};
    EXPECT_EQ(*std::prev(tTokenList.end()), tLastEntry);
}

TEST(AutoList, SuccessfulParseSingleEntries)
{
    const auto tToken1 = std::string_view{"birdo"};
    const auto [tResult, tSuccess] = test_utilities::parse_input<StringListType>(tToken1);
    EXPECT_TRUE(tSuccess);
    EXPECT_EQ(tResult.mList.front().mToken, tToken1);
}

TEST(AutoList, SuccessfulParseMultipleEntries)
{
    const auto tToken1 = std::string{"one"};
    const auto tToken2 = std::string{"two"};
    const auto tToken3 = std::string{"three"};
    const auto tAllTokens = std::vector<std::string>{tToken1, tToken2, tToken3};
    const auto tTokenList = utilities::concatenate_container(tAllTokens, ", ");

    const auto [tResult, tSuccess] = test_utilities::parse_input<UserDefinedTokenListType>(tTokenList);
    EXPECT_TRUE(tSuccess);

    ASSERT_EQ(tResult.mList.size(), tAllTokens.size());
    EXPECT_EQ(tResult.mList.front().mToken, tAllTokens.front());
    EXPECT_EQ(tResult.mList.back().mToken, tAllTokens.back());
}

TEST(AutoList, SuccessfulParseMultipleBounds)
{
    const auto tToken1 = Bounds{0, 1};
    const auto tToken2 = Bounds{-1, 1};
    const auto tToken3 = Bounds{2, 3};
    const auto tAllTokens = std::vector<Bounds>{tToken1, tToken2, tToken3};

    const auto tTokenList = std::string_view{"[0,1], [-1,1], [2,3]"};

    const auto [tResult, tSuccess] = test_utilities::parse_input<BoundsListType>(tTokenList);
    EXPECT_TRUE(tSuccess);

    ASSERT_EQ(tResult.mList.size(), tAllTokens.size());
    EXPECT_EQ(tResult.mList.front().mLower, tToken1.mLower);
    EXPECT_EQ(tResult.mList.back().mLower, tToken3.mLower);
}

}  // namespace plato::input_parser::unittest
