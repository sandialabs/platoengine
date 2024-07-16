#include <gtest/gtest.h>

#include <string_view>

#include "Test_Helpers.hpp"
#include "plato/input_parser/FileList.hpp"
#include "plato/input_parser/UserDefinedToken.hpp"

namespace plato::input_parser
{
namespace
{
struct LowerCaseWithUnderscore
{
    constexpr const char* operator()() const { return "a-z_"; }
};

using LowerCaseWithUnderscoreToken = UserDefinedToken<LowerCaseWithUnderscore>;

template <typename ValidChars>
[[nodiscard]] std::pair<UserDefinedToken<ValidChars>, bool> parse_input(const std::string_view aInput)
{
    namespace bs = boost::spirit;
    using Iterator = std::string_view::const_iterator;
    using Rule = bs::qi::rule<Iterator, UserDefinedToken<ValidChars>(), bs::ascii::space_type>;

    auto tData = UserDefinedToken<ValidChars>{};
    const Rule tTestRule = bs::qi::auto_ >> (bs::qi::eol | bs::qi::eoi);
    auto tIter = aInput.cbegin();
    const bool tParseResult = phrase_parse(tIter, aInput.cend(), tTestRule, bs::ascii::space, tData);
    return {tData, tParseResult};
}

}  // namespace

TEST(UserDefinedToken, ValidChars)
{
    const auto tToken = UserDefinedToken<LowerCaseWithUnderscore>{};
    EXPECT_EQ(tToken.kValidChars, LowerCaseWithUnderscore{}());
}

TEST(UserDefinedToken, Insert)
{
    auto tUserDefinedToken = UserDefinedToken<LowerCaseWithUnderscore>{};
    EXPECT_TRUE(tUserDefinedToken.mToken.empty());

    // Check insert
    tUserDefinedToken.insert(tUserDefinedToken.mToken.end(), 'a');
    EXPECT_EQ(tUserDefinedToken.mToken, "a");
    tUserDefinedToken.insert(tUserDefinedToken.mToken.end(), 'b');
    EXPECT_EQ(tUserDefinedToken.mToken, "ab");
    tUserDefinedToken.insert(tUserDefinedToken.mToken.begin(), 'b');
    EXPECT_EQ(tUserDefinedToken.mToken, "bab");
}

TEST(UserDefinedToken, Iterators)
{
    constexpr std::string_view tTestString = "foo_bar";
    auto tUserDefinedToken = UserDefinedToken<LowerCaseWithUnderscore>{std::string{tTestString}};
    EXPECT_EQ(tUserDefinedToken.mToken, tTestString.data());

    std::string tCopy;
    std::copy(tUserDefinedToken.begin(), tUserDefinedToken.end(), std::back_inserter(tCopy));
    EXPECT_EQ(tCopy, tTestString.data());
}

TEST(UserDefinedToken, ConstIterators)
{
    constexpr std::string_view tTestString = "const_foo_bar";
    const auto tUserDefinedTokenConst = UserDefinedToken<LowerCaseWithUnderscore>{std::string{tTestString}};
    EXPECT_EQ(tUserDefinedTokenConst.mToken, tTestString.data());

    std::string tCopy;
    std::copy(tUserDefinedTokenConst.begin(), tUserDefinedTokenConst.end(), std::back_inserter(tCopy));
    EXPECT_EQ(tCopy, tTestString.data());
}

TEST(UserDefinedToken, ParseFail)
{
    const auto tTestString = std::string_view{"1_invalid_input_0"};

    const auto [tResult, tSuccess] = parse_input<LowerCaseWithUnderscore>(tTestString);
    EXPECT_FALSE(tSuccess);
}

TEST(UserDefinedToken, ParseSuccess)
{
    const auto tTestString = std::string_view{"valid_input"};

    const auto [tResult, tSuccess] = parse_input<LowerCaseWithUnderscore>(tTestString);
    EXPECT_TRUE(tSuccess);
    EXPECT_EQ(tResult.mToken, tTestString);
}

}  // namespace plato::input_parser
