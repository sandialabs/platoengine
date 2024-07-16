#ifndef PLATO_SERVICES_PLUGINDIRECTORYPATH
#define PLATO_SERVICES_PLUGINDIRECTORYPATH

#include <filesystem>
#include <optional>

namespace plato::services
{
/// @brief Returns the plugin directory path in the installation directory, which contains all installed shared
/// libraries.
/// @note Returns `std::nullopt` if the expected plugin path does not exist.
std::optional<std::filesystem::path> plugin_directory_path();
}  // namespace plato::services

#endif
