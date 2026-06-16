#include <gtest/gtest.h>

#include <iterator>
#include <regex>
#include <sstream>
#include <string>

#include "plato/documentation/HelpDocumentationRegistration.hpp"
#include "plato/test_utilities/TestContext.hpp"
#include "plato/utilities/NamedType.hpp"

namespace plato::documentation::unittest
{

namespace
{
using Key = utilities::NamedType<std::string, struct KeyTag>;

auto help_documentation_function_one() -> ComponentDocumentation
{
    const InputKeywordDocumentation tCommand{"name_1", "type_1", "comment_1"};
    return {.mDescription = "description_1", .mInputs = {tCommand}};
}

auto help_documentation_function_two() -> ComponentDocumentation
{
    const InputKeywordDocumentation tCommand{"name_2", "type_2", "comment_2"};
    return {.mDescription = "description_2", .mInputs = {tCommand, tCommand}};
}

[[maybe_unused]] const auto kNewHelpDocumentationRegistrationOne =
    HelpDocumentationRegistration{"key_one", []() { return help_documentation_function_one(); }};

[[maybe_unused]] const auto kNewHelpDocumentationRegistrationTwo =
    HelpDocumentationRegistration{"key_two", []() { return help_documentation_function_two(); }};

}  // namespace

TEST(HelpDocumentationRegistration, GenerateDocumentation)
{
    const auto tDocumentation = generate_documentation();

    EXPECT_GE(tDocumentation.size(), 2U);

    ASSERT_EQ(tDocumentation.at("key_one").mInputs.size(), 1U);
    EXPECT_EQ(tDocumentation.at("key_one").mDescription, "description_1");
    EXPECT_EQ(tDocumentation.at("key_one").mInputs.front().mType, "type_1");

    ASSERT_EQ(tDocumentation.at("key_two").mInputs.size(), 2U);
    EXPECT_EQ(tDocumentation.at("key_two").mDescription, "description_2");
    EXPECT_EQ(tDocumentation.at("key_two").mInputs.front().mComment, "comment_2");
    EXPECT_EQ(tDocumentation.at("key_two").mInputs.back().mName, "name_2");
}

namespace
{
void check_for_key(const Key& aKey,
                   const std::string& aString,
                   const unsigned int aCount,
                   const test_utilities::TestContext& aTestContext)
{
    const std::regex tPattern(aKey.mValue);
    auto tRegexIterator = std::sregex_iterator(aString.begin(), aString.end(), tPattern);
    EXPECT_EQ(std::distance(tRegexIterator, std::sregex_iterator()), aCount) << aTestContext << "String: \n" << aString;
}
}  // namespace

TEST(HelpDocumentationRegistration, KnownInputs)
{
    std::stringstream tStringStream;
    known_inputs(tStringStream);
    const auto tString = tStringStream.str();

    check_for_key(Key{"description_1"}, tString, 1U, TEST_CONTEXT("Description one"));
    check_for_key(Key{"key_one"}, tString, 1U, TEST_CONTEXT("Key one"));
    check_for_key(Key{"type_1"}, tString, 1U, TEST_CONTEXT("Key type_1"));
    check_for_key(Key{"comment_1"}, tString, 1U, TEST_CONTEXT("Key comment_1"));

    check_for_key(Key{"description_2"}, tString, 1U, TEST_CONTEXT("Description two"));
    check_for_key(Key{"key_two"}, tString, 1U, TEST_CONTEXT("Key two"));
    check_for_key(Key{"type_2"}, tString, 2U, TEST_CONTEXT("Key type_2"));
    check_for_key(Key{"comment_2"}, tString, 2U, TEST_CONTEXT("Key comment_2"));
}

}  // namespace plato::documentation::unittest
