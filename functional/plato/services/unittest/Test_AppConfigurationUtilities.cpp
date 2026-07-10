#include <gtest/gtest.h>

#include <vector>

#include "plato/services/AppConfiguration.hpp"
#include "plato/services/AppConfigurationUtilities.hpp"

namespace plato::services::unittest
{
TEST(AppConfigurationUtilities, SharedLibraryPath)
{
    const auto tTestPath = std::filesystem::path{"/fake/path/to/"};
    const auto tTestLibName = std::filesystem::path{"libpatterns.so"};
    const auto tPaisleyCriterion = CriterionConfiguration{
        .mName = "paisley", .mIsParallelized = false, .mIsScalar = true, .mFunctionName = "paisley_function"};
    const auto tConfiguration = AppConfiguration{
        .mName = "patterns", .mLibraryFileName = tTestLibName.string(), .mCriteria = {tPaisleyCriterion}};
    EXPECT_EQ(tTestPath / tTestLibName, shared_library_path(AppConfigurationWithDirectory{tConfiguration, tTestPath}));
}

TEST(AppConfigurationUtilities, SharedLibraryData)
{
    const auto tTestPath = std::filesystem::path{"/fake/path/to/"};
    const auto tTestLibName = std::filesystem::path{"libpatterns.so"};
    const auto tTestLoadFunctionName = std::filesystem::path{"load_patterns"};
    const auto tTestUnloadFunctionName = std::filesystem::path{"unload_patterns"};

    {
        const auto tConfiguration =
            AppConfiguration{.mName = "patterns", .mLibraryFileName = tTestLibName.string(), .mCriteria = {}};
        const auto tData = shared_library_data(AppConfigurationWithDirectory{tConfiguration, tTestPath});
        EXPECT_EQ(tTestPath / tTestLibName, tData.mSharedLibraryPath) << "Empty functions";
        EXPECT_FALSE(tData.mLibraryLoadFunctionName.has_value()) << "Empty functions";
        EXPECT_FALSE(tData.mLibraryShutdownFunctionName.has_value()) << "Empty functions";
    }
    {
        const auto tConfiguration = AppConfiguration{.mName = "patterns",
                                                     .mLibraryFileName = tTestLibName.string(),
                                                     .mCriteria = {},
                                                     .mLibraryLoadFunctionName = tTestLoadFunctionName};
        const auto tData = shared_library_data(AppConfigurationWithDirectory{tConfiguration, tTestPath});
        ASSERT_TRUE(tData.mLibraryLoadFunctionName.has_value()) << "Load function";
        EXPECT_EQ(tData.mLibraryLoadFunctionName.value(), tTestLoadFunctionName) << "Load function";
        EXPECT_FALSE(tData.mLibraryShutdownFunctionName.has_value()) << "Load function";
    }
    {
        const auto tConfiguration = AppConfiguration{.mName = "patterns",
                                                     .mLibraryFileName = tTestLibName.string(),
                                                     .mCriteria = {},
                                                     .mLibraryLoadFunctionName = std::nullopt,
                                                     .mLibraryShutdownFunctionName = tTestUnloadFunctionName};
        const auto tData = shared_library_data(AppConfigurationWithDirectory{tConfiguration, tTestPath});
        ASSERT_TRUE(tData.mLibraryShutdownFunctionName.has_value()) << "Unload function";
        EXPECT_EQ(tData.mLibraryShutdownFunctionName.value(), tTestUnloadFunctionName) << "Unload function";
        EXPECT_FALSE(tData.mLibraryLoadFunctionName.has_value()) << "Unload function";
    }
}

TEST(AppConfigurationUtilities, CriterionConfigurationWithName)
{
    const auto tArgyleCriterion = CriterionConfiguration{
        .mName = "argyle", .mIsParallelized = false, .mIsScalar = true, .mFunctionName = "argyle_function"};
    const auto tPaisleyCriterion = CriterionConfiguration{
        .mName = "paisley", .mIsParallelized = false, .mIsScalar = true, .mFunctionName = "paisley_function"};
    const auto tPatternsConfiguration = AppConfiguration{
        .mName = "patterns", .mLibraryFileName = "testlib.so", .mCriteria = {tPaisleyCriterion, tArgyleCriterion}};

    const auto tChucksCriterion = CriterionConfiguration{
        .mName = "chucks", .mIsParallelized = false, .mIsScalar = true, .mFunctionName = "chucks_function"};
    const auto tVansCriterion = CriterionConfiguration{
        .mName = "vans", .mIsParallelized = false, .mIsScalar = true, .mFunctionName = "vans_function"};
    const auto tShoesConfiguration = AppConfiguration{
        .mName = "shoes", .mLibraryFileName = "testlib.so", .mCriteria = {tChucksCriterion, tVansCriterion}};

    const auto tAppConfigurations = std::vector{
        AppConfigurationWithDirectory{.mConfiguration = tPatternsConfiguration, .mLibraryDirectory = "testlib.so"},
        AppConfigurationWithDirectory{.mConfiguration = tShoesConfiguration, .mLibraryDirectory = "testlib.so"}};

    {
        const auto tCriterion = criterion_configuration_with_name({.mAppName = "patterns", .mCriterionName = "paisley"},
                                                                  tAppConfigurations);
        ASSERT_TRUE(tCriterion);
        EXPECT_EQ(tCriterion->mName, "paisley");
    }
    {
        const auto tCriterion =
            criterion_configuration_with_name({.mAppName = "shoes", .mCriterionName = "vans"}, tAppConfigurations);
        ASSERT_TRUE(tCriterion);
        EXPECT_EQ(tCriterion->mName, "vans");
    }
    {
        const auto tCriterion = criterion_configuration_with_name(
            {.mAppName = "shoes", .mCriterionName = "air-jordans"}, tAppConfigurations);
        EXPECT_FALSE(tCriterion);
    }
}

}  // namespace plato::services::unittest
