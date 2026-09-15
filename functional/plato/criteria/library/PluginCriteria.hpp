#ifndef PLATO_CRITERIA_LIBRARY_PLUGINCRITERIA
#define PLATO_CRITERIA_LIBRARY_PLUGINCRITERIA

#include <filesystem>
#include <vector>

namespace plato::criteria::library
{
/// @brief Registers all apps found in the directories given in @a aSearchDirectories
/// @note Apps that are already registered are ignored.
/// @return A list of strings of the apps registered on this call (not in total).
auto register_plugin_apps(const std::vector<std::filesystem::path>& aSearchDirectories) -> std::vector<std::string>;
}  // namespace plato::criteria::library

#endif
