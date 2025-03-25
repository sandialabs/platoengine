#include <gtest/gtest.h>

#include <fstream>

#include "plato/test_utilities/TestDirectorySetupTeardown.hpp"

namespace plato::test_utilities::unittest
{
TEST(TestDirectorySetupTeardown, CtorDtor)
{
    const auto tDirectoryPath = std::filesystem::path{"milky-way"};
    {
        const auto tDirectory = TestDirectorySetupTeardown{tDirectoryPath};
        EXPECT_EQ(tDirectoryPath, tDirectory.directory());
        EXPECT_TRUE(std::filesystem::exists(tDirectory.directory()));
    }
    EXPECT_FALSE(std::filesystem::exists(tDirectoryPath));
}

TEST(TestDirectorySetupTeardown, WriteFunction)
{
    auto tWriteFunctionCalled = false;
    const auto tWriteFunction = [&tWriteFunctionCalled](const std::filesystem::path& aPath)
    {
        auto tFileStream = std::ofstream{aPath};
        tFileStream << "Hello, world!\n";
        tWriteFunctionCalled = true;
    };

    const auto tDirectoryPath = std::filesystem::path{"pinwheel"};
    const auto tDirectory = TestDirectorySetupTeardown{tDirectoryPath};
    const auto tFilePath = std::filesystem::path{"sn1909a"};
    tDirectory.writeFile(tWriteFunction, tFilePath);

    EXPECT_TRUE(std::filesystem::exists(tDirectoryPath / tFilePath));
    EXPECT_TRUE(tWriteFunctionCalled);
}

TEST(TestDirectorySetupTeardown, MoveCtor)
{
    auto tWriteCount = 0U;
    const auto tWriteFunction = [&tWriteCount](const std::filesystem::path&) { ++tWriteCount; };

    const auto tFileName = std::filesystem::path{"pa-99-n2"};
    const auto tDirectoryPath = std::filesystem::path{"andromeda"};
    {
        auto tDirectory1 = TestDirectorySetupTeardown(tDirectoryPath);
        const auto tDirectory2 = std::move(tDirectory1);

        tDirectory1.writeFile(tWriteFunction, tFileName);
        auto tExpectedWriteCount = 0U;
        EXPECT_EQ(tWriteCount, tExpectedWriteCount);  // No increment to tWriteCount on call to moved-from object

        tDirectory2.writeFile(tWriteFunction, tFileName);
        ++tExpectedWriteCount;
        EXPECT_EQ(tWriteCount, tExpectedWriteCount);
    }

    EXPECT_FALSE(std::filesystem::exists(tDirectoryPath));
}

TEST(TestDirectorySetupTeardown, MoveAssignment)
{
    auto tWriteCount = 0U;
    const auto tWriteFunction = [&tWriteCount](const std::filesystem::path&) { ++tWriteCount; };

    const auto tFileName = std::filesystem::path{"M51-ULS-1b"};
    const auto tDirectoryPath1 = std::filesystem::path{"whirlpool"};
    const auto tDirectoryPath2 = std::filesystem::path{"tadpole"};
    {
        auto tDirectory1 = TestDirectorySetupTeardown(tDirectoryPath1);
        auto tDirectory2 = TestDirectorySetupTeardown(tDirectoryPath2);
        tDirectory1 = std::move(tDirectory2);

        tDirectory1.writeFile(tWriteFunction, tFileName);
        auto tExpectedWriteCount = 1U;
        EXPECT_EQ(tWriteCount, tExpectedWriteCount);  // Move assignment swaps, so still expect a write call

        tDirectory2.writeFile(tWriteFunction, tFileName);
        ++tExpectedWriteCount;
        EXPECT_EQ(tWriteCount, tExpectedWriteCount);
    }
    EXPECT_FALSE(std::filesystem::exists(tDirectoryPath1));
    EXPECT_FALSE(std::filesystem::exists(tDirectoryPath2));
}

}  // namespace plato::test_utilities::unittest
