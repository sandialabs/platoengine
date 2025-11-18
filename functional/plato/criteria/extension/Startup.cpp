#include "plato/criteria/extension/Startup.hpp"

#include <format>

#include "plato/criteria/extension/PluginCriteria.hpp"
#include "plato/services/PluginDirectoryPath.hpp"
#include "plato/services/SystemLogger.hpp"
#include "plato/utilities/OptionalToVector.hpp"

namespace plato::criteria::extension
{

auto startup() -> std::size_t
{
    static const auto tNumberOfPluginsLoaded =
        register_plugin_apps(utilities::optional_to_vector(services::plugin_directory_path()));

    auto tLogger = services::system_logger();
    tLogger.logInfo(std::format("Registered {} criteria plugins", tNumberOfPluginsLoaded));

    return tNumberOfPluginsLoaded;
}

}  // namespace plato::criteria::extension
