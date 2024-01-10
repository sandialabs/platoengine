#include <gtest/gtest.h>

#include "Plato_SuppressBoostNvccWarnings.hpp"

#include "Plato_FileList.hpp"

TEST(FileName, Insert)
{
    Plato::FileName tFileName;
    EXPECT_TRUE(tFileName.mName.empty());

    // Check insert
    tFileName.insert(tFileName.mName.end(), 'a');
    EXPECT_EQ(tFileName.mName, "a");
    tFileName.insert(tFileName.mName.end(), 'b');
    EXPECT_EQ(tFileName.mName, "ab");
    tFileName.insert(tFileName.mName.begin(), 'b');
    EXPECT_EQ(tFileName.mName, "bab");
}

TEST(FileName, Iterators)
{
    constexpr std::string_view tTestString = "foo_bar";
    Plato::FileName tFileName{std::string{tTestString}};
    EXPECT_EQ(tFileName.mName, tTestString.data());

    std::string tCopy;
    std::copy(tFileName.begin(), tFileName.end(), std::back_inserter(tCopy));
    EXPECT_EQ(tCopy, tTestString.data());
}

TEST(FileName, ConstIterators)
{
    constexpr std::string_view tTestString = "const_foo_bar";
    const Plato::FileName tFileNameConst{std::string{tTestString}};
    EXPECT_EQ(tFileNameConst.mName, tTestString.data());

    std::string tCopy;
    std::copy(tFileNameConst.begin(), tFileNameConst.end(), std::back_inserter(tCopy));
    EXPECT_EQ(tCopy, tTestString.data());
}

TEST(FileList, Insert)
{
    Plato::FileList tFileList;
    EXPECT_TRUE(tFileList.mList.empty());

    // Check insert
    tFileList.insert(tFileList.mList.end(), "a");
    ASSERT_EQ(tFileList.mList.size(), 1);
    EXPECT_EQ(tFileList.mList.front(), "a");
    tFileList.insert(tFileList.mList.end(), "b");
    ASSERT_EQ(tFileList.mList.size(), 2);
    EXPECT_EQ(tFileList.mList.front(), "a");
    EXPECT_EQ(tFileList.mList.back(), "b");
    tFileList.insert(tFileList.mList.begin(), "b");
    ASSERT_EQ(tFileList.mList.size(), 3);
    EXPECT_EQ(tFileList.mList.at(0), "b");
    EXPECT_EQ(tFileList.mList.at(1), "a");
    EXPECT_EQ(tFileList.mList.at(2), "b");
}

TEST(FileList, Iterators)
{
    Plato::FileList tFileList{{"r", "a", "w", "r"}};
    std::vector<std::string> tCopy;
    std::copy(tFileList.begin(), tFileList.end(), std::back_inserter(tCopy));
    ASSERT_EQ(tCopy.size(), 4);
    EXPECT_EQ(tFileList.mList.at(0), "r");
    EXPECT_EQ(tFileList.mList.at(1), "a");
    EXPECT_EQ(tFileList.mList.at(2), "w");
    EXPECT_EQ(tFileList.mList.at(3), "r");
}

TEST(FileList, ConstIterators)
{
    const Plato::FileList tFileList{{"r", "a", "w", "r"}};
    std::vector<std::string> tCopy;
    std::copy(tFileList.begin(), tFileList.end(), std::back_inserter(tCopy));
    ASSERT_EQ(tCopy.size(), 4);
    EXPECT_EQ(tFileList.mList.at(0), "r");
    EXPECT_EQ(tFileList.mList.at(1), "a");
    EXPECT_EQ(tFileList.mList.at(2), "w");
    EXPECT_EQ(tFileList.mList.at(3), "r");
}

#include "Plato_RestoreBoostNvccWarnings.hpp"
