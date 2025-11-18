#ifndef PLATO_INPUT_PARSER_TEST_UTILITIES_TESTHELPERS
#define PLATO_INPUT_PARSER_TEST_UTILITIES_TESTHELPERS

#include <gtest/gtest.h>

#include <boost/optional.hpp>
#include <boost/optional/optional_io.hpp>
#include <boost/spirit/include/qi.hpp>
#include <string>
#include <vector>

#include "plato/input_parser/FileList.hpp"
#include "plato/test_utilities/TestContext.hpp"

namespace plato::input_parser
{
struct ParsedInput;
}

namespace plato::input_parser::test_utilities
{
/// @brief Tests that @a aOptionalValue contains a value via assertion, and that it is equal to @a aVal.
template <typename T, typename U>
void test_existence_and_equality(const boost::optional<T>& aOptionalVal, const U& aVal)
{
    ASSERT_TRUE(aOptionalVal);
    EXPECT_EQ(aOptionalVal.value(), aVal);
}

template <typename ValidChars>
inline void test_existence_and_equality(const boost::optional<UserDefinedToken<ValidChars>>& aOptionalVal,
                                        const std::string& aVal)
{
    ASSERT_TRUE(aOptionalVal);
    EXPECT_EQ(aOptionalVal.value().mToken, aVal);
}

template <>
inline void test_existence_and_equality(const boost::optional<FileList>& aOptionalVal,
                                        const std::vector<std::string>& aVal)
{
    ASSERT_TRUE(aOptionalVal);
    EXPECT_EQ(aOptionalVal.value().list().mList, aVal);
}

template <typename T>
void copy_test(T& aFileList, const plato::test_utilities::TestContext& aTestContext)
{
    std::vector<std::string> tCopy;
    std::copy(aFileList.begin(), aFileList.end(), std::back_inserter(tCopy));
    EXPECT_EQ(aFileList.list().mList, tCopy) << aTestContext;
}

/// @brief Helper for parsing individual entries, such as UserDefinedToken or UserDefinedTokenList.
template <typename ParsedType>
[[nodiscard]] std::pair<ParsedType, bool> parse_input(const std::string_view aInput)
{
    namespace bs = boost::spirit;
    using Iterator = std::string_view::const_iterator;
    using Rule = bs::qi::rule<Iterator, ParsedType(), bs::ascii::space_type>;

    auto tData = ParsedType{};
    const Rule tTestRule = bs::qi::auto_ >> (bs::qi::eol | bs::qi::eoi);
    const auto* tIter = aInput.cbegin();
    const bool tParseResult = phrase_parse(tIter, aInput.cend(), tTestRule, bs::ascii::space, tData);
    return {tData, tParseResult};
}

}  // namespace plato::input_parser::test_utilities

#endif
