#include "plato/test_utilities/FilesystemTestUtility.hpp"

#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>

#include "plato/test_utilities/TestContext.hpp"

namespace plato::test_utilities
{

void test_for_existence_and_remove(const std::vector<std::filesystem::path>& aFilesToCheck,
                                   const TestContext& aTestContext)
{
    for (const auto& tFileName : aFilesToCheck)
    {
        EXPECT_TRUE(std::filesystem::exists(tFileName)) << aTestContext << ": " << tFileName;
        EXPECT_TRUE(std::filesystem::remove(tFileName)) << aTestContext << ": " << tFileName;
    }
}

auto file_to_string(const std::filesystem::path& aPath) -> std::string
{
    auto tFile = std::ifstream{aPath};
    auto tFileContents = std::stringstream{};
    tFile >> tFileContents.rdbuf();
    return tFileContents.str();
}

}  // namespace plato::test_utilities
