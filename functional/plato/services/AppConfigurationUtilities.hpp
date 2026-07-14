#ifndef PLATO_SERVICES_APPCONFIGURATIONUTILITIES
#define PLATO_SERVICES_APPCONFIGURATIONUTILITIES

#include <filesystem>

#include "plato/services/AppConfiguration.hpp"
#include "plato/services/SharedLibrarySetupTeardown.hpp"

namespace plato::services
{
/// @brief Helper type for associating an app name with a criterion name.
struct CriterionName
{
    std::string mAppName;
    std::string mCriterionName;
};

/// @brief Returns all app configurations found in the directory at paths @a aSearchDirectories.
[[nodiscard]] auto app_configurations(const std::vector<std::filesystem::path>& aSearchDirectories)
    -> std::vector<AppConfigurationWithDirectory>;

/// @brief Adds the directory @a aDirectory to the app configuration for generating absolute paths.
[[nodiscard]] auto app_configuration_with_directory(AppConfiguration aAppConfiguration,
                                                    std::filesystem::path aDirectory) -> AppConfigurationWithDirectory;

/// @brief Returns all app configurations found in the plugins directory.
[[nodiscard]] auto plugin_configurations() -> std::vector<AppConfigurationWithDirectory>;

/// @brief Returns the path to the shared library contained in @a aAppConfiguration.
[[nodiscard]] auto shared_library_path(const AppConfigurationWithDirectory& aAppConfiguration) -> std::filesystem::path;

/// @brief Returns the shared library data contained in @a aAppConfiguration.
///
/// This includes the shared library path and load/shutdown function names.
[[nodiscard]] auto shared_library_data(const AppConfigurationWithDirectory& aAppConfiguration) -> SharedLibraryData;

/// @brief Returns the CriterionRegistration with name @a aCriterionName.
[[nodiscard]] auto criterion_configuration_with_name(
    const CriterionName& aCriterionName,
    const std::vector<AppConfigurationWithDirectory>& aAppConfigurations) -> std::optional<CriterionConfiguration>;

/// @brief A helper function object for writing an AppConfiguration to disk.
struct AppConfigurationWriter
{
    void operator()(const std::filesystem::path& aFilePath) const;
    AppConfiguration mAppConfiguration{};
};
}  // namespace plato::services

#endif
