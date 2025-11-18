#include <gtest/gtest.h>

#include "plato/criteria/extension/Startup.hpp"
#include "plato/services/AppConfigurationUtilities.hpp"
#include "plato/services/PluginDirectoryPath.hpp"
#include "plato/utilities/OptionalToVector.hpp"

namespace plato::criteria::extension::unittest
{
TEST(PluginCriteria, NumberOfPluginsRegistered)
{
    const auto tNumberOfPluginsLoaded = startup();
    const auto tNumberOfAppConfigurations =
        services::app_configurations(utilities::optional_to_vector(services::plugin_directory_path())).size();
    EXPECT_EQ(tNumberOfAppConfigurations, tNumberOfPluginsLoaded);
}
}  // namespace plato::criteria::extension::unittest
