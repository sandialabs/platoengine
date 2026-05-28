#include <gtest/gtest.h>

#include <algorithm>
#include <filesystem>
#include <iterator>
#include <vector>

#include "plato/test_utilities/TestContext.hpp"
#include "plato/utilities/FileUtilities.hpp"
#include "plato/utilities/NamedReference.hpp"

namespace plato::utilities::unittest
{
namespace
{
using TestPath = utilities::NamedReference<const std::filesystem::path, struct TestPathTag>;
using UniquePath = utilities::NamedReference<const std::filesystem::path, struct UniquePathTag>;

void test_filename_and_path(const TestPath& aTestPath,
                            const UniquePath& aUniquePath,
                            const test_utilities::TestContext& aTestContext)
{
    EXPECT_NE(aTestPath.mValue, aUniquePath.mValue) << aTestContext;
    // The parent path should be unchanged, only the filename itself should change
    EXPECT_EQ(aTestPath.mValue.get().parent_path(), aUniquePath.mValue.get().parent_path()) << aTestContext;
    EXPECT_NE(aTestPath.mValue.get().filename(), aUniquePath.mValue.get().filename()) << aTestContext;
}

void test_unique_filename(const std::filesystem::path& aTestPath, const test_utilities::TestContext& aTestContext)
{
    const auto tUniquePath = make_filename_unique(aTestPath);
    test_filename_and_path(TestPath{aTestPath}, UniquePath{tUniquePath},
                           EXTEND_CONTEXT("Unique file vs original", aTestContext));
    // Make many more and check that they're all different from the previous
    constexpr auto tNumFilesToCheck = unsigned{100};
    auto tUniquePaths = std::vector<std::filesystem::path>{};
    tUniquePaths.reserve(tNumFilesToCheck);
    std::generate_n(std::back_inserter(tUniquePaths), tNumFilesToCheck,
                    [&aTestPath]() { return make_filename_unique(aTestPath); });
    for (const auto& tCurrentPath : tUniquePaths)
    {
        test_filename_and_path(TestPath{aTestPath}, UniquePath{tCurrentPath},
                               EXTEND_CONTEXT("Unique file vs. original", aTestContext));
        test_filename_and_path(TestPath{tUniquePath}, UniquePath{tCurrentPath},
                               EXTEND_CONTEXT("Unique file vs. another unique file", aTestContext));
    }
}
}  // namespace

TEST(FileUtilities, UniqueFilenameFullPath) { test_unique_filename("some/path/file.txt", TEST_CONTEXT("Full path")); }

TEST(FileUtilities, UniqueFilenameNoExtension)
{
    test_unique_filename("some/path/file", TEST_CONTEXT("No extension with path"));
}

TEST(FileUtilities, UniqueFilenameNoPath) { test_unique_filename("file.exo", TEST_CONTEXT("No path, only filename")); }

TEST(FileUtilities, UniqueFilenameNoFilename)
{
    const auto tTestPath = std::filesystem::path{"some/path/"};
    const auto tUniquePath = make_filename_unique(tTestPath);
    EXPECT_EQ(tTestPath, tUniquePath);
    EXPECT_FALSE(tUniquePath.has_filename());
}

}  // namespace plato::utilities::unittest
