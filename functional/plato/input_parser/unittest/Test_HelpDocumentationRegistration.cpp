#include <gtest/gtest.h>

#include <regex>
#include <sstream>

#include "plato/input_parser/HelpDocumentationRegistration.hpp"
#include "plato/test_utilities/TestContext.hpp"

namespace plato::input_parser
{

namespace
{

auto help_documentation_function_one() -> Documentation
{
    HelpDocumentation tCommand{"name_1", "type_1", "comment_1"};
    return {tCommand};
}

auto help_documentation_function_two() -> Documentation
{
    HelpDocumentation tCommand{"name_2", "type_2", "comment_2"};
    return {tCommand, tCommand};
}

[[maybe_unused]] static auto kNewHelpDocumentationRegistrationOne =
    HelpDocumentationRegistration{"key_one", []() { return help_documentation_function_one(); }};

[[maybe_unused]] static auto kNewHelpDocumentationRegistrationTwo =
    HelpDocumentationRegistration{"key_two", []() { return help_documentation_function_two(); }};

}  // namespace

TEST(HelpDocumentationRegistration, GenerateDocumentation)
{
    const auto tDocumentation = generate_documentation();
    EXPECT_GE(tDocumentation.size(), 3U);
    ASSERT_EQ(tDocumentation.at("key_one").size(), 1U);
    EXPECT_EQ(tDocumentation.at("key_one").front().mType, "type_1");
    ASSERT_EQ(tDocumentation.at("key_two").size(), 2U);
    EXPECT_EQ(tDocumentation.at("key_two").front().mComment, "comment_2");
    EXPECT_EQ(tDocumentation.at("key_two").back().mName, "name_2");
}

void check_for_key(const std::string& aKey,
                   const std::string& aString,
                   const unsigned int aCount,
                   const test_utilities::TestContext& aTestContext)
{
    std::regex tPattern(aKey);
    auto tRegexIterator = std::sregex_iterator(aString.begin(), aString.end(), tPattern);
    EXPECT_EQ(std::distance(tRegexIterator, std::sregex_iterator()), aCount) << aTestContext;
}

TEST(HelpDocumentationRegistration, KnownInputs)
{
    std::stringstream tStringStream;
    known_inputs(tStringStream);
    const auto tString = tStringStream.str();

    check_for_key("key_one", tString, 1U, TEST_CONTEXT("Key one"));
    check_for_key("type_1", tString, 1U, TEST_CONTEXT("Key type_1"));
    check_for_key("comment_1", tString, 1U, TEST_CONTEXT("Key comment_1"));
    check_for_key("key_two", tString, 1U, TEST_CONTEXT("Key two"));
    check_for_key("type_2", tString, 2U, TEST_CONTEXT("Key type_2"));
    check_for_key("comment_2", tString, 2U, TEST_CONTEXT("Key comment_2"));
}

}  // namespace plato::input_parser
