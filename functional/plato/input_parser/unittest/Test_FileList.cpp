#include <gtest/gtest.h>

#include "plato/input_parser/FileList.hpp"
#include "plato/input_parser/UserDefinedTokenList.hpp"
#include "plato/input_parser/test_utilities/TestHelpers.hpp"
#include "plato/test_utilities/TestContext.hpp"

namespace plato::input_parser::unittest
{
TEST(FileList, Insert)
{
    FileList tFileList;
    EXPECT_TRUE(tFileList.list().mList.empty());

    // Check insert
    tFileList.insert(tFileList.end(), "a");
    ASSERT_EQ(tFileList.size(), 1U);
    EXPECT_EQ(tFileList.list().mList.front(), "a");
    tFileList.insert(tFileList.end(), "b");
    ASSERT_EQ(tFileList.size(), 2U);
    EXPECT_EQ(tFileList.list().mList.front(), "a");
    EXPECT_EQ(tFileList.list().mList.back(), "b");
    tFileList.insert(tFileList.begin(), "b");
    ASSERT_EQ(tFileList.size(), 3U);
    EXPECT_EQ(tFileList.list().mList.at(0), "b");
    EXPECT_EQ(tFileList.list().mList.at(1), "a");
    EXPECT_EQ(tFileList.list().mList.at(2), "b");
}

TEST(FileList, Iterators)
{
    auto tFileList = FileList{{{"arbitrary", "nonexistent", "file", "names"}}};
    test_utilities::copy_test(tFileList, TEST_CONTEXT("Iterators"));
}

TEST(FileList, ConstIterators)
{
    const auto tFileList = FileList{{{"definitely", "meaningless", "strings"}}};
    test_utilities::copy_test(tFileList, TEST_CONTEXT("Const iterators"));
}

TEST(FileList, DataTypeNames)
{
    EXPECT_EQ(data_type_documentation(FileName{}), "`<file name>`");
    EXPECT_EQ(data_type_documentation(FileList{}), "Comma separated list of `<file name>`");
}

}  // namespace plato::input_parser::unittest
