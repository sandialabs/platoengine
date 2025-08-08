#include <gtest/gtest.h>

#include "plato/input_parser/UserDefinedTokenList.hpp"
#include "plato/input_parser/unittest/Test_Helpers.hpp"

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

TEST(UserDefinedTokenList, StreamInsertion)
{
    const auto tToken1 = std::string{"racoon"};
    const auto tToken2 = std::string{"frog"};
    const auto tTokenList = UserDefinedTokenList<LowerCaseOnly>{{{tToken1, tToken2}}};

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
    EXPECT_EQ(tResult, std::vector{tToken1});
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

    EXPECT_EQ(tResult, tAllTokens);
}

TEST(UserDefinedTokenList, FailedParseBadCharacters)
{
    const auto tBadInput = std::string_view{"dry-bones, koopa-troopa"};
    const auto [tResult, tSuccess] = parse_input<ListType>(tBadInput);
    EXPECT_FALSE(tSuccess);
}

}  // namespace plato::input_parser::unittest
