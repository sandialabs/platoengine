#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>

#include "plato/services/AppConfiguration.hpp"
#include "plato/services/AppConfigurationUtilities.hpp"
#include "plato/test_utilities/TestContext.hpp"
#include "plato/test_utilities/TestDirectorySetupTeardown.hpp"

namespace plato::services
{
namespace
{
const auto kTestCriterionConfiguration = CriterionConfiguration{
    .mName = "test-criterion", .mIsParallelized = true, .mIsScalar = true, .mFunctionName = "plato_test_criterion"};

const auto kAnotherTestCriterionConfiguration = CriterionConfiguration{.mName = "another_test-criterion",
                                                                       .mIsParallelized = false,
                                                                       .mIsScalar = false,
                                                                       .mFunctionName = "plato_another_test_criterion"};

const auto kVectorTestCriterionConfiguration =
    CriterionConfiguration{.mName = "vector_test-criterion",
                           .mIsParallelized = false,
                           .mIsScalar = false,
                           .mFunctionName = "plato_vector_test_criterion",
                           .mVectorComponentNames = std::vector<std::string>{"octopus", "cuttlefish"}};

const auto kTestConfiguration =
    AppConfiguration{.mName = "test-app", .mLibraryFileName = "libtest.so", .mCriteria = {kTestCriterionConfiguration}};

const auto kAnotherTestConfiguration =
    AppConfiguration{.mName = "another-test-app",
                     .mLibraryFileName = "libanothertest.so",
                     .mCriteria = {kTestCriterionConfiguration, kAnotherTestCriterionConfiguration}};

void testSerializeRoundTrip(const AppConfiguration& aSerializable, const test_utilities::TestContext& aTestContext)
{
    const auto tFilename = std::filesystem::path{"out.config"};
    save_configuration(aSerializable, tFilename);
    const auto tRoundTripResult = load_configuration(tFilename);

    EXPECT_EQ(aSerializable, tRoundTripResult) << aTestContext;

    std::filesystem::remove(tFilename);
}

}  // namespace

TEST(AppConfiguration, Serialization)
{
    const auto tAppConfiguration = services::AppConfiguration{
        /*.mName=*/"test-app",
        /*.mLibraryFileName=*/"libtest.so",
        /*.mCriteria=*/
        {kTestCriterionConfiguration, kAnotherTestCriterionConfiguration, kVectorTestCriterionConfiguration}};
    testSerializeRoundTrip(tAppConfiguration, TEST_CONTEXT("App configuration"));
}

TEST(AppConfiguration, AppConfigurationWithDirectory)
{
    const auto tSharedLibName = std::string_view{"libappetizer.so"};
    const auto tAppConfiguration =
        services::AppConfiguration{/*.mName=*/"appetizer", /*.mLbraryFileName=*/std::string{tSharedLibName},
                                   /*.mCriteria=*/{kTestCriterionConfiguration}};
    const auto tDirectory = std::filesystem::path{"/path/to/food"};

    const auto tAppConfigurationWithDirectory = app_configuration_with_directory(tAppConfiguration, tDirectory);
    EXPECT_EQ(tAppConfigurationWithDirectory.mConfiguration, tAppConfiguration);
    EXPECT_EQ(tAppConfigurationWithDirectory.mLibraryDirectory, tDirectory);
    EXPECT_EQ(shared_library_path(tAppConfigurationWithDirectory), tDirectory / tSharedLibName);
}

TEST(AppConfiguration, AppConfigurations)
{
    const auto tConfigurationTempDirectory = test_utilities::TestDirectorySetupTeardown{"test-configuration-directory"};
    tConfigurationTempDirectory.writeFile(services::AppConfigurationWriter{kTestConfiguration}, "test-1.config")
        .writeFile(services::AppConfigurationWriter{kAnotherTestConfiguration}, "test-2.config");
    const auto tAppConfigurations = app_configurations({tConfigurationTempDirectory.directory()});

    EXPECT_EQ(tAppConfigurations.size(), 2u);

    const auto tTestConfigurationIter = std::find_if(
        tAppConfigurations.cbegin(), tAppConfigurations.cend(), [](const auto& aAppConfigurationWithDirectory)
        { return kTestConfiguration == aAppConfigurationWithDirectory.mConfiguration; });
    EXPECT_NE(tTestConfigurationIter, tAppConfigurations.cend());

    const auto tAnotherTestConfigurationIter = std::find_if(
        tAppConfigurations.cbegin(), tAppConfigurations.cend(), [](const auto& aAppConfigurationWithDirectory)
        { return kAnotherTestConfiguration == aAppConfigurationWithDirectory.mConfiguration; });
    EXPECT_NE(tAnotherTestConfigurationIter, tAppConfigurations.cend());
}

}  // namespace plato::services
