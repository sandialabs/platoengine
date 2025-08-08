#include <gtest/gtest.h>

#include "plato/input_parser/FileList.hpp"
#include "plato/input_parser/UserDefinedTokenList.hpp"
#include "plato/input_parser/unittest/Test_Helpers.hpp"

namespace plato::input_parser::unittest
{
TEST(FileList, Insert)
{
    FileList tFileList;
    EXPECT_TRUE(tFileList.list().mList.empty());

    // Check insert
    tFileList.insert(tFileList.end(), "a");
    ASSERT_EQ(tFileList.size(), 1u);
    EXPECT_EQ(tFileList.list().mList.front(), "a");
    tFileList.insert(tFileList.end(), "b");
    ASSERT_EQ(tFileList.size(), 2u);
    EXPECT_EQ(tFileList.list().mList.front(), "a");
    EXPECT_EQ(tFileList.list().mList.back(), "b");
    tFileList.insert(tFileList.begin(), "b");
    ASSERT_EQ(tFileList.size(), 3u);
    EXPECT_EQ(tFileList.list().mList.at(0), "b");
    EXPECT_EQ(tFileList.list().mList.at(1), "a");
    EXPECT_EQ(tFileList.list().mList.at(2), "b");
}

TEST(FileList, Iterators)
{
    FileList tFileList{{{"r", "a", "w", "r"}}};
    copy_test(tFileList, TEST_CONTEXT("Iterators"));
}

TEST(FileList, ConstIterators)
{
    const FileList tFileList{{{"r", "a", "w", "r"}}};
    copy_test(tFileList, TEST_CONTEXT("Const iterators"));
}
}  // namespace plato::input_parser::unittest
