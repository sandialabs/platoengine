#ifndef PLATO_SERVICES_APPCONFIGURATIONUTILITIES
#define PLATO_SERVICES_APPCONFIGURATIONUTILITIES

#include <filesystem>

#include "plato/services/AppConfiguration.hpp"

namespace plato::services
{
/// @brief Helper type for associating an app name with a criterion name.
struct CriterionName
{
    std::string mAppName;
    std::string mCriterionName;
};

/// @brief Returns all app configurations found in the directory at paths @a aSearchDirectories.
[[nodiscard]] std::vector<AppConfigurationWithDirectory> app_configurations(
    const std::vector<std::filesystem::path>& aSearchDirectories);

/// @brief Adds the directory @a aDirectory to the app configuration for generating absolute paths.
[[nodiscard]] AppConfigurationWithDirectory app_configuration_with_directory(AppConfiguration aAppConfiguration,
                                                                             std::filesystem::path aDirectory);

/// @brief Returns all app configurations found in the plugins directory.
[[nodiscard]] auto plugin_configurations() -> std::vector<AppConfigurationWithDirectory>;

/// @brief Returns the path to the shared library contained in @a aAppConfiguration.
[[nodiscard]] std::filesystem::path shared_library_path(const AppConfigurationWithDirectory& aAppConfiguration);

/// @brief Returns the CriterionRegistration with name @a aCriterionName.
/// @note The name of the criterion must be prefixed with the app name, separated by a colon, e.g.,
/// "platoanalyze:mass_properties".
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
