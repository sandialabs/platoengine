#include "plato/services/AppConfigurationUtilities.hpp"

#include <algorithm>
#include <filesystem>
#include <iterator>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "plato/services/AppConfiguration.hpp"
#include "plato/services/PluginDirectoryPath.hpp"
#include "plato/utilities/OptionalToVector.hpp"
#include "plato/utilities/TransformIf.hpp"

namespace plato::services
{
namespace
{
constexpr auto kConfigFileExtension = std::string_view{".config"};

auto criterion_configuration_with_name(const std::string& aCriterionName,
                                       const std::vector<CriterionConfiguration>& aCriterionConfigurations)
    -> std::optional<CriterionConfiguration>
{
    const auto tCriterion =
        std::ranges::find_if(aCriterionConfigurations, [&aCriterionName](const auto& aCriterionConfiguration)
                             { return aCriterionConfiguration.mName == aCriterionName; });
    if (tCriterion != aCriterionConfigurations.end())
    {
        return *tCriterion;
    }
    return std::nullopt;
}

}  // namespace

auto app_configurations(const std::vector<std::filesystem::path>& aSearchDirectories)
    -> std::vector<AppConfigurationWithDirectory>
{
    auto tAppConfigurations = std::vector<AppConfigurationWithDirectory>{};
    for (const auto& tDirectory : aSearchDirectories)
    {
        utilities::transform_if(
            std::filesystem::directory_iterator{tDirectory}, std::back_inserter(tAppConfigurations),
            [&tDirectory](const auto& aDirectoryEntry)
            { return app_configuration_with_directory(load_configuration(aDirectoryEntry.path()), tDirectory); },
            [](const auto& aDirectoryEntry) { return aDirectoryEntry.path().extension() == kConfigFileExtension; });
    }
    return tAppConfigurations;
}

auto app_configuration_with_directory(AppConfiguration aAppConfiguration,
                                      std::filesystem::path aDirectory) -> AppConfigurationWithDirectory
{
    return AppConfigurationWithDirectory{.mConfiguration = std::move(aAppConfiguration),
                                         .mLibraryDirectory = std::move(aDirectory)};
}

auto plugin_configurations() -> std::vector<AppConfigurationWithDirectory>
{
    return app_configurations(utilities::optional_to_vector(services::plugin_directory_path()));
}

auto shared_library_path(const AppConfigurationWithDirectory& aAppConfiguration) -> std::filesystem::path
{
    return aAppConfiguration.mLibraryDirectory / aAppConfiguration.mConfiguration.mLibraryFileName;
}

auto shared_library_data(const AppConfigurationWithDirectory& aAppConfiguration) -> SharedLibraryData
{
    return {.mSharedLibraryPath = shared_library_path(aAppConfiguration),
            .mLibraryLoadFunctionName = aAppConfiguration.mConfiguration.mLibraryLoadFunctionName,
            .mLibraryShutdownFunctionName = aAppConfiguration.mConfiguration.mLibraryShutdownFunctionName};
}

auto criterion_configuration_with_name(const CriterionName& aCriterionName,
                                       const std::vector<AppConfigurationWithDirectory>& aAppConfigurations)
    -> std::optional<CriterionConfiguration>
{
    const auto tAppConfiguration =
        std::ranges::find_if(aAppConfigurations, [&aCriterionName](const auto& aAppConfiguration)
                             { return aAppConfiguration.mConfiguration.mName == aCriterionName.mAppName; });
    if (tAppConfiguration != aAppConfigurations.end())
    {
        return criterion_configuration_with_name(aCriterionName.mCriterionName,
                                                 tAppConfiguration->mConfiguration.mCriteria);
    }
    return std::nullopt;
}

void AppConfigurationWriter::operator()(const std::filesystem::path& aFilePath) const
{
    save_configuration(mAppConfiguration, aFilePath);
}

}  // namespace plato::services
