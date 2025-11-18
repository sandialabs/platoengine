#include <gtest/gtest.h>

#include <string_view>

#include "plato/input_parser/UserDefinedToken.hpp"
#include "plato/input_parser/test_utilities/TestHelpers.hpp"

namespace plato::input_parser::unittest
{
namespace
{
struct LowerCaseWithUnderscore
{
    constexpr const char* operator()() const { return "a-z_"; }
};

using LowerCaseWithUnderscoreToken = UserDefinedToken<LowerCaseWithUnderscore>;
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

    const auto [tResult, tSuccess] = test_utilities::parse_input<LowerCaseWithUnderscoreToken>(tTestString);
    EXPECT_FALSE(tSuccess);
}

TEST(UserDefinedToken, ParseSuccess)
{
    const auto tTestString = std::string_view{"valid_input"};

    const auto [tResult, tSuccess] = test_utilities::parse_input<LowerCaseWithUnderscoreToken>(tTestString);
    EXPECT_TRUE(tSuccess);
    EXPECT_EQ(tResult.mToken, tTestString);
}

}  // namespace plato::input_parser::unittest
