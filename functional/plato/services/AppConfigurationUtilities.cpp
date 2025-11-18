#include "plato/services/AppConfigurationUtilities.hpp"

#include <algorithm>
#include <filesystem>

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

std::vector<AppConfigurationWithDirectory> app_configurations(
    const std::vector<std::filesystem::path>& aSearchDirectories)
{
    auto tAppConfigurations = std::vector<AppConfigurationWithDirectory>{};
    for (const auto& tDirectory : aSearchDirectories)
    {
        utilities::transform_if(
            std::filesystem::directory_iterator{tDirectory}, std::back_inserter(tAppConfigurations),
            [&tDirectory](const auto& tDirectoryEntry)
            { return app_configuration_with_directory(load_configuration(tDirectoryEntry.path()), tDirectory); },
            [](const auto& tDirectoryEntry) { return tDirectoryEntry.path().extension() == kConfigFileExtension; });
    }
    return tAppConfigurations;
}

AppConfigurationWithDirectory app_configuration_with_directory(AppConfiguration aAppConfiguration,
                                                               std::filesystem::path aDirectory)
{
    return AppConfigurationWithDirectory{/*.mConfiguration=*/std::move(aAppConfiguration),
                                         /*.mLibraryDirectory=*/std::move(aDirectory)};
}

auto plugin_configurations() -> std::vector<AppConfigurationWithDirectory>
{
    return app_configurations(utilities::optional_to_vector(services::plugin_directory_path()));
}

std::filesystem::path shared_library_path(const AppConfigurationWithDirectory& aAppConfiguration)
{
    return aAppConfiguration.mLibraryDirectory / aAppConfiguration.mConfiguration.mLibraryFileName;
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
