#include <gtest/gtest.h>

#include "plato/input_parser/BlockStructRule.hpp"
#include "plato/input_parser/InputBlocks.hpp"
#include "plato/input_parser/unittest/Test_Helpers.hpp"
#include "plato/test_utilities/TestContext.hpp"

namespace plato::input_parser::unittest
{
namespace
{
void check_nothing_parsed(const ParsedInput& aInput)
{
    EXPECT_TRUE(aInput.mObjectives.empty());
    EXPECT_TRUE(aInput.mConstraints.empty());
    EXPECT_FALSE(aInput.mROLOptimization);
}

ParsedInput parse_and_check_success(const std::string& aInput, const test_utilities::TestContext& aTestContext)
{
    const auto [tParseResult, tIter, tData] = parse_string(aInput);

    EXPECT_TRUE(tParseResult) << aTestContext;
    EXPECT_EQ(tIter, aInput.cend()) << aTestContext;
    return tData;
}

}  // namespace

TEST(InputBlockStruct, BlockName)
{
    const std::string tResult = block_name<brick_shape_geometry>();
    EXPECT_EQ(tResult, "brick_shape_geometry");
}

TEST(ParsedInput, MisspelledBegin)
{
    const std::string tInput =
        R"(
          begn constraint mp_constraint
            active true
          end
       )";

    // Parse
    const auto [tParseResult, tIter, tData] = parse_string(tInput);

    // Tests
    EXPECT_TRUE(tParseResult);
    EXPECT_NE(tIter, tInput.cend());
    check_nothing_parsed(tData);
}

TEST(ParsedInput, MisspelledEnd)
{
    const std::string tInput =
        R"(
          begin constraint mp_constraint
            active true
          en
       )";

    // Parse
    const auto [tParseResult, tIter, tData] = parse_string(tInput);

    // Tests
    EXPECT_FALSE(tParseResult);
    EXPECT_NE(tIter, tInput.cend());
    check_nothing_parsed(tData);
}

TEST(ParsedInput, MisspelledBlockType)
{
    const std::string tInput =
        R"(
          begin constrain mp_constraint
            active true
          end
       )";

    // Parse
    const auto [tParseResult, tIter, tData] = parse_string(tInput);

    // Tests
    EXPECT_TRUE(tParseResult);
    EXPECT_NE(tIter, tInput.cend());
    check_nothing_parsed(tData);
}

TEST(ParsedInput, MisspelledToken)
{
    const std::string tInput =
        R"(
          begin constraint mp_constraint
            ave true
          end
       )";

    // Parse
    const auto [tParseResult, tIter, tData] = parse_string(tInput);

    // Tests
    EXPECT_FALSE(tParseResult);
    EXPECT_NE(tIter, tInput.cend());
    check_nothing_parsed(tData);
}

TEST(ParsedInput, MisspelledValue)
{
    const std::string tInput =
        R"(
          begin constraint mp_constraint
            active te
          end
       )";

    // Parse
    const auto [tParseResult, tIter, tData] = parse_string(tInput);

    // Tests
    EXPECT_FALSE(tParseResult);
    EXPECT_NE(tIter, tInput.cend());
    check_nothing_parsed(tData);
}

TEST(ParsedInput, MissingValue)
{
    const std::string tInput =
        R"(
          begin constraint mp_constraint
            number_of_processors 10
            active
          end
       )";

    // Parse
    const auto [tParseResult, tIter, tData] = parse_string(tInput);

    // Tests
    EXPECT_FALSE(tData.mConstraints.front().active);
    EXPECT_TRUE(tData.mConstraints.front().number_of_processors);
    EXPECT_EQ(tData.mConstraints.front().number_of_processors.value(), 10u);
}

TEST(ParsedInput, CommentWithinLine)
{
    const std::string tInput =
        R"(
          begin objective objective_1 
            active false # true
            app nodal_sum
          end
       )";

    const auto tParsedInput = parse_and_check_success(tInput, TEST_CONTEXT("Commented out input mid-line"));

    ASSERT_EQ(tParsedInput.mObjectives.size(), 1u);
    test_existence_and_equality(tParsedInput.mObjectives.front().active, false);
    test_existence_and_equality(tParsedInput.mObjectives.front().app, std::string{"nodal_sum"});
}

TEST(ParsedInput, CommentEntireLine)
{
    const std::string tInput =
        R"(
          begin objective objective_1 
            # app nodal_sum
            app volume
          end
       )";

    const auto tParsedInput = parse_and_check_success(tInput, TEST_CONTEXT("Commented out input"));

    ASSERT_EQ(tParsedInput.mObjectives.size(), 1u);
    test_existence_and_equality(tParsedInput.mObjectives.front().app, std::string{"volume"});
}

TEST(ParsedInput, CommentNonInput)
{
    const std::string tInput =
        R"(
          begin objective objective_1 
            # This is not real input!
            app volume
          end
       )";
    const auto tParsedInput = parse_and_check_success(tInput, TEST_CONTEXT("Comment on non-input"));

    ASSERT_EQ(tParsedInput.mObjectives.size(), 1u);
    test_existence_and_equality(tParsedInput.mObjectives.front().app, std::string{"volume"});
}

TEST(ParsedInput, CommentMultipleLinesAndCharacters)
{
    const std::string tInput =
        R"(
          begin objective objective_1 
            ## This is not # real input!
            # active false
            app volume
          end
       )";
    const auto tParsedInput = parse_and_check_success(tInput, TEST_CONTEXT("Comment multiple lines and characters"));

    ASSERT_EQ(tParsedInput.mObjectives.size(), 1u);
    test_existence_and_equality(tParsedInput.mObjectives.front().app, std::string{"volume"});
    EXPECT_FALSE(tParsedInput.mObjectives.front().active.has_value());
}

}  // namespace plato::input_parser::unittest
