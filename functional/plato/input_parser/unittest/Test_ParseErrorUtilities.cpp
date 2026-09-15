#include <gtest/gtest.h>

#include <algorithm>
#include <boost/spirit/home/support/common_terminals.hpp>
#include <boost/spirit/include/qi.hpp>  // NOLINT(misc-include-cleaner)
#include <string>
#include <utility>
#include <vector>

#include "plato/components/ComponentType.hpp"
#include "plato/documentation/DataTypeDocumentation.hpp"
#include "plato/input_parser/BlockStructKeywordForms.hpp"
#include "plato/input_parser/InputBlockStruct.hpp"
#include "plato/input_parser/ParseErrorUtilities.hpp"
#include "plato/test_utilities/Strings.hpp"
#include "plato/test_utilities/TestContext.hpp"
#include "plato/utilities/StringUtilities.hpp"

// clang-format off
PLATO_INPUT_BLOCK_STRUCT(
    (plato)(input_parser), ParseErrorUtilitiesTestBlock, plato::components::ComponentType::kFilter,
    "A test block for parse error utilities.",
    (int, peon_count, "Number of peons.")
    (double, lumber_cost, "Amount of lumber.")
)
// clang-format on

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
    {
        const auto tErrorMessage = error_message(tParsedToIterator, tInput.cend(), tTokenDelimeter);
        EXPECT_FALSE(tErrorMessage.empty());
        plato::test_utilities::expect_string_contains_substring(tErrorMessage, "bc",
                                                                TEST_CONTEXT("Parser error message"));
    }
    {
        const auto tBlockName = std::string{"Zugzug"};
        const auto tErrorMessage = error_message(tParsedToIterator, tInput.cend(), tTokenDelimeter, tBlockName);
        EXPECT_FALSE(tErrorMessage.empty());
        plato::test_utilities::expect_string_contains_substring(tErrorMessage, tBlockName,
                                                                TEST_CONTEXT("Parser error message with block name"));
    }
}

TEST(ParseErrorUtilities, InputBlockErrorMessage)
{
    const auto tPeonCount = std::string{"peon_count"};
    const auto tLumberCost = std::string{"lumber_cost"};
    const auto tBadToken = std::string{"not_an_int"};
    const auto tInput = utilities::concatenate(tPeonCount, " ", tBadToken, " ", tLumberCost, " 2.0");

    const auto tParsedToIterator = std::search(tInput.begin(), tInput.end(), tBadToken.begin(), tBadToken.end());
    const auto tTokenDelimeter = ' ';
    const auto tErrorMessage =
        input_block_error_message<ParseErrorUtilitiesTestBlock>(tParsedToIterator, tInput.cend(), tTokenDelimeter);

    EXPECT_FALSE(tErrorMessage.empty());

    plato::test_utilities::expect_string_contains_substring(
        tErrorMessage, "Parsing error near:", TEST_CONTEXT("Input block error message includes parse context header"));

    plato::test_utilities::expect_string_contains_substring(
        tErrorMessage, tBadToken, TEST_CONTEXT("Input block error message includes local parse context"));

    plato::test_utilities::expect_string_contains_substring(
        tErrorMessage, detail::closest_keyword_form_syntax(),
        TEST_CONTEXT("Input block error message includes expected keyword form header"));

    plato::test_utilities::expect_string_contains_substring(
        tErrorMessage, tPeonCount, TEST_CONTEXT("Input block error message includes first keyword"));

    plato::test_utilities::expect_string_contains_substring(
        tErrorMessage, tLumberCost, TEST_CONTEXT("Input block error message includes second keyword"));

    plato::test_utilities::expect_string_contains_substring(
        tErrorMessage, documentation::data_type_documentation(int{}),
        TEST_CONTEXT("Input block error message includes int type documentation"));

    plato::test_utilities::expect_string_contains_substring(
        tErrorMessage, documentation::data_type_documentation(double{}),
        TEST_CONTEXT("Input block error message includes double type documentation"));
}

}  // namespace plato::input_parser::unittest
