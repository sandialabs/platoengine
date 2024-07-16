#ifndef PLATO_CRITERIA_EXTENSION_PLUGINCRITERIA
#define PLATO_CRITERIA_EXTENSION_PLUGINCRITERIA

#include <filesystem>
#include <vector>

namespace plato::criteria::extension
{
/// @brief Returns the total number of plugins found and registered at program startup time.
[[nodiscard]] std::size_t number_of_plugins_registered_at_startup();

/// @brief Registers all apps found in the directories given in @a aSearchDirectories
/// @note Apps that are already registered are ignored.
/// @return The number of apps registered on this call (not in total).
std::size_t register_plugin_apps(const std::vector<std::filesystem::path>& aSearchDirectories);
}  // namespace plato::criteria::extension

#endif
