#include "plato/criteria/extension/Startup.hpp"

#include <cstddef>
#include <format>

#include "plato/criteria/library/PluginCriteria.hpp"
#include "plato/services/PluginDirectoryPath.hpp"
#include "plato/services/SystemLogger.hpp"
#include "plato/utilities/OptionalToVector.hpp"
#include "plato/utilities/StringUtilities.hpp"

namespace plato::criteria::extension
{

auto startup() -> std::size_t
{
    static const auto tLoadedPluginNames =
        library::register_plugin_apps(utilities::optional_to_vector(services::plugin_directory_path()));

    auto tLogger = services::system_logger();
    tLogger.logInfo(std::format("Registered {} criteria plugins: {}", tLoadedPluginNames.size(),
                                utilities::concatenate_container(tLoadedPluginNames, ", ")));

    return tLoadedPluginNames.size();
}

}  // namespace plato::criteria::extension
