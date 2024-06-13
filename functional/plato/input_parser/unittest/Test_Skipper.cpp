#include <gtest/gtest.h>

#include <string_view>

#include "plato/input_parser/Skipper.hpp"
#include "plato/test_utilities/TestContext.hpp"

namespace plato::input_parser::unittest
{
namespace
{
void check_parsed_integers(const std::string_view aTestText,
                           const std::vector<int>& aExpectedData,
                           const test_utilities::TestContext& aTestContext)
{
    auto tTestIter = aTestText.cbegin();
    const auto tIntParser = *boost::spirit::qi::int_;
    std::vector<int> tData;
    const auto tSkipper = SkipperRule<std::string_view::const_iterator>{};
    const auto parsed =
        boost::spirit::qi::phrase_parse(tTestIter, aTestText.cend(), tIntParser, tSkipper.skipperRule(), tData);
    EXPECT_TRUE(parsed) << aTestContext;
    EXPECT_EQ(tTestIter, aTestText.cend()) << aTestContext;
    EXPECT_EQ(tData.size(), aExpectedData.size()) << aTestContext;
    EXPECT_EQ(tData, aExpectedData) << aTestContext;
}
}  // namespace

TEST(Skipper, SkipsWhiteSpace)
{
    constexpr auto tTestText = std::string_view{"1 2 \t 3 -1     10\n30"};
    const auto tExpectedData = std::vector{1, 2, 3, -1, 10, 30};
    check_parsed_integers(tTestText, tExpectedData, TEST_CONTEXT("Whitespace only"));
}

TEST(Skipper, SkipsComments)
{
    constexpr auto tTestText = std::string_view{
        R"(1 2  3 -1  10
        # 50
        30 # 42)"};
    const auto tExpectedData = std::vector{1, 2, 3, -1, 10, 30};
    check_parsed_integers(tTestText, tExpectedData, TEST_CONTEXT("With comments"));
}

}  // namespace plato::input_parser::unittest