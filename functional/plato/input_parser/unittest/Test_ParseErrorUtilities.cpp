#include <gtest/gtest.h>

#include <boost/spirit/include/qi.hpp>
#include <utility>

#include "plato/input_parser/ParseErrorUtilities.hpp"
#include "plato/test_utilities/Strings.hpp"

namespace plato::input_parser::unittest
{
namespace
{
template <typename ParsedType, typename Parser>
auto parse(const std::string& aInput, const Parser& aParser)
{
    namespace bs = boost::spirit;
    namespace bsq = bs::qi;

    auto tInputIterator = aInput.begin();
    auto tData = ParsedType{};
    const auto tParserSuccess = bsq::phrase_parse(tInputIterator, aInput.cend(), aParser, bs::ascii::space, tData);
    return std::make_pair(tParserSuccess, tInputIterator);
}

}  // namespace

TEST(ParseErrorUtilities, ParserHasError)
{
    namespace bsq = boost::spirit::qi;

    {
        const auto tInput = std::string{"a, b, c"};
        const auto [tParseSuccess, tParsedToIterator] = parse<std::vector<char>>(tInput, bsq::char_ % ',');
        EXPECT_FALSE(parser_has_error(tParseSuccess, tParsedToIterator, tInput.cend()));
    }
    {
        const auto tInput = std::string{"a, 42, c"};
        const auto [tParseSuccess, tParsedToIterator] = parse<std::vector<char>>(tInput, bsq::char_ % ',');
        EXPECT_TRUE(parser_has_error(tParseSuccess, tParsedToIterator, tInput.cend()));
    }
}

TEST(ParseErrorUtilities, ErrorMessage)
{
    namespace bsq = boost::spirit::qi;
    const auto tInput = std::string{"x, abc, y"};
    const auto [tParseSuccess, tParsedToIterator] = parse<std::vector<char>>(tInput, bsq::char_ % ',');

    const auto tTokenDelimeter = ',';
    const auto tErrorMessage = error_message(tParsedToIterator, tInput.cend(), tTokenDelimeter);
    EXPECT_FALSE(tErrorMessage.empty());

    const auto tFullString = std::string{tParsedToIterator, tInput.cend()};
    plato::test_utilities::expect_string_contains_substring(tErrorMessage, "bc", TEST_CONTEXT("Parser error message"));
}

}  // namespace plato::input_parser::unittest
