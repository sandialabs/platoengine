#include <gtest/gtest.h>

#include "plato/input_parser/FileList.hpp"
#include "plato/input_parser/InputBlockStruct.hpp"
#include "plato/input_parser/unittest/Test_Helpers.hpp"

// clang-format off
PLATO_INPUT_BLOCK_STRUCT(
    (plato)(input_parser), TestFileTypes,
    (plato::input_parser::FileName, file_name)
    (plato::input_parser::FileList, file_list)
)
// clang-format on

namespace plato::input_parser::unittest
{
TEST(ParseFileTypes, FileName)
{
    TestFileTypes tTestBlock;
    tTestBlock.file_name = FileName{"wolverine"};
    test_existence_and_equality(tTestBlock.file_name, std::string{"wolverine"});
}

TEST(ParseFileTypes, ParseFileList)
{
    TestFileTypes tTestBlock;
    tTestBlock.file_list = FileList{{"storm", "cyclops"}};
    test_existence_and_equality(tTestBlock.file_list, std::vector<std::string>{"storm", "cyclops"});
}
}  // namespace plato::input_parser::unittest