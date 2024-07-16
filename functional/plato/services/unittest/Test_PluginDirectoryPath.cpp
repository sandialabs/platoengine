#include <gtest/gtest.h>

#include <filesystem>

#include "plato/services/PluginDirectoryPath.hpp"

namespace plato::services::unittest
{
TEST(PluginDirectoryPath, CheckPath)
{
    // This is a little difficult to test since we don't know if the installation path exists or not,
    // which depends on if this is run before or after installation.
    const auto tPluginPath = plugin_directory_path();
    if (tPluginPath)
    {
        EXPECT_TRUE(std::filesystem::exists(tPluginPath.value()));
        EXPECT_TRUE(std::filesystem::is_directory(tPluginPath.value()));
        EXPECT_EQ(tPluginPath->filename(), "plugins");
    }
    else
    {
        std::cout << "No installation path, skipping test." << std::endl;
    }
}
}  // namespace plato::services::unittest
