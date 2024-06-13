#include "plato/test_utilities/FilesystemTestUtility.hpp"

#include <gtest/gtest.h>

#include <filesystem>

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
}  // namespace plato::test_utilities
