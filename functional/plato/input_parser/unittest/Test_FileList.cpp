#include <gtest/gtest.h>

#include "plato/input_parser/FileList.hpp"
#include "plato/input_parser/unittest/Test_Helpers.hpp"

namespace plato::input_parser::unittest
{
TEST(FileList, Insert)
{
    FileList tFileList;
    EXPECT_TRUE(tFileList.mList.empty());

    // Check insert
    tFileList.insert(tFileList.mList.end(), "a");
    ASSERT_EQ(tFileList.mList.size(), 1u);
    EXPECT_EQ(tFileList.mList.front(), "a");
    tFileList.insert(tFileList.mList.end(), "b");
    ASSERT_EQ(tFileList.mList.size(), 2u);
    EXPECT_EQ(tFileList.mList.front(), "a");
    EXPECT_EQ(tFileList.mList.back(), "b");
    tFileList.insert(tFileList.mList.begin(), "b");
    ASSERT_EQ(tFileList.mList.size(), 3u);
    EXPECT_EQ(tFileList.mList.at(0), "b");
    EXPECT_EQ(tFileList.mList.at(1), "a");
    EXPECT_EQ(tFileList.mList.at(2), "b");
}

TEST(FileList, Iterators)
{
    FileList tFileList{{"r", "a", "w", "r"}};
    copy_test(tFileList, TEST_CONTEXT("Iterators"));
}

TEST(FileList, ConstIterators)
{
    const FileList tFileList{{"r", "a", "w", "r"}};
    copy_test(tFileList, TEST_CONTEXT("Const iterators"));
}
}  // namespace plato::input_parser::unittest