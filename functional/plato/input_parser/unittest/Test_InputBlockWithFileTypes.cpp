#include <gtest/gtest.h>

#include "plato/input_parser/FileList.hpp"
#include "plato/input_parser/InputBlockStruct.hpp"
#include "plato/input_parser/InputFieldTypes.hpp"
#include "plato/input_parser/test_utilities/TestHelpers.hpp"

// clang-format off
PLATO_INPUT_BLOCK_STRUCT(
    (plato)(input_parser), TestFileTypes, plato::components::ComponentType::kProcessManager,
    (plato::input_parser::FileName, file_name, "help")
    (plato::input_parser::FileList, file_list, "help")
    (plato::input_parser::IdentifierString, exodus_name, "help")
)
// clang-format on

namespace plato::input_parser::unittest
{
TEST(ParseFileTypes, FileName)
{
    TestFileTypes tTestBlock;
    tTestBlock.file_name = FileName{"wolverine"};
    test_utilities::test_existence_and_equality(tTestBlock.file_name, std::string{"wolverine"});
}

TEST(ParseFileTypes, ParseFileList)
{
    TestFileTypes tTestBlock;
    tTestBlock.file_list = FileList{{"storm", "cyclops"}};
    test_utilities::test_existence_and_equality(tTestBlock.file_list, std::vector<std::string>{"storm", "cyclops"});
}

TEST(ParseIdentifierString, ParseFail)
{
    const auto tTestString = std::string_view{"1_invalid._input_0"};
    const auto [tResult, tSuccess] = test_utilities::parse_input<IdentifierString>(tTestString);
    EXPECT_FALSE(tSuccess);
}

TEST(ParseIdentifierString, ParseSuccess)
{
    const auto tTestString = std::string_view{"Valid-09_Input"};
    const auto [tResult, tSuccess] = test_utilities::parse_input<IdentifierString>(tTestString);
    EXPECT_TRUE(tSuccess);
    EXPECT_EQ(tResult.mToken, tTestString);
}

}  // namespace plato::input_parser::unittest
