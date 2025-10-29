#include "plato/criteria/library/test_utilities/TestAppConfigurations.hpp"

#include <string_view>

#include "plato/services/AppConfiguration.hpp"
#include "plato/services/AppConfigurationUtilities.hpp"

namespace plato::criteria::library::test_utilities
{
namespace
{
constexpr auto kTestAppName = std::string_view{"continents"};

void write_app_config(const std::string_view& aAppName,
                      const plato::test_utilities::TestDirectorySetupTeardown& aConfigurationTempDirectory)
{
    const auto tCriterionSerialConfiguration = services::CriterionConfiguration{
        .mName = "scalar", .mIsParallelized = false, .mIsScalar = true, .mFunctionName = "test_scalar_criterion"};
    const auto tCriterionVectorConfiguration =
        services::CriterionConfiguration{.mName = "vector-no-components",
                                         .mIsParallelized = false,
                                         .mIsScalar = false,
                                         .mFunctionName = "test_vector_no_components_criterion"};
    const auto tCriterionVectorWithComponentsConfiguration = services::CriterionConfiguration{
        .mName = "vector-with-components",
        .mIsParallelized = false,
        .mIsScalar = false,
        .mFunctionName = "test_vector_with_components_criterion",
        .mVectorComponents = std::map<std::size_t, std::string>{{0U, "america"}, {1U, "europe"}, {2U, "asia"}}};

    auto tAppConfiguration =
        services::AppConfiguration{.mName = std::string{aAppName},
                                   .mLibraryFileName = std::filesystem::path{"phony-library.so"},
                                   .mCriteria = {tCriterionSerialConfiguration, tCriterionVectorConfiguration,
                                                 tCriterionVectorWithComponentsConfiguration}};
    aConfigurationTempDirectory.writeFile(services::AppConfigurationWriter{std::move(tAppConfiguration)},
                                          "test-mass-app.config");
}

}  // namespace

auto test_configurations() -> std::pair<plato::test_utilities::TestDirectorySetupTeardown, std::string_view>
{
    const auto tTestPluginDirectory = std::filesystem::path{"test-plugin-directory"};
    auto tConfigurationTempDirectory =
        plato::test_utilities::TestDirectorySetupTeardown{tTestPluginDirectory, boost::mpi::communicator{}};
    write_app_config(kTestAppName, tConfigurationTempDirectory);
    return std::make_pair(std::move(tConfigurationTempDirectory), kTestAppName);
}

}  // namespace plato::criteria::library::test_utilities
