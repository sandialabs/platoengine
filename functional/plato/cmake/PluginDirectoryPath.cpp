#include "plato/services/PluginDirectoryPath.hpp"

#include <iostream>

namespace plato::services
{

std::optional<std::filesystem::path> plugin_directory_path()
{
    static const auto tPluginPath = std::filesystem::path{"@FUNCTIONAL_PLUGIN_INSTALL_PATH@"};
    if (std::filesystem::exists(tPluginPath))
    {
        return tPluginPath;
    }
    return std::nullopt;
}

}  // namespace plato::services
