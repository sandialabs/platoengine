#include "plato/services/AppConfigurationUtilities.hpp"

#include <algorithm>
#include <filesystem>

#include "plato/services/PluginDirectoryPath.hpp"
#include "plato/utilities/TransformIf.hpp"

namespace plato::services
{
namespace
{
constexpr auto kConfigFileExtension = std::string_view{".config"};
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

std::filesystem::path shared_library_path(const AppConfigurationWithDirectory& aAppConfiguration)
{
    return aAppConfiguration.mLibraryDirectory / aAppConfiguration.mConfiguration.mLibraryFileName;
}

void AppConfigurationWriter::operator()(const std::filesystem::path& aFilePath) const
{
    save_configuration(mAppConfiguration, aFilePath);
}

}  // namespace plato::services
