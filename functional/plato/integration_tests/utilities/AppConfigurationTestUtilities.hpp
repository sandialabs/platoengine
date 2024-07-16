#ifndef PLATO_INTEGRATION_TESTS_UTILITIES_APPCONFIGURATIONTESTUTILITIES
#define PLATO_INTEGRATION_TESTS_UTILITIES_APPCONFIGURATIONTESTUTILITIES

#include "plato/services/AppConfiguration.hpp"

namespace plato::integration_tests::utilities
{
/// @brief Returns an AppConfiguration that can be used for testing a shared lib at path @a aSharedLibPath.
///
/// The configuration will have an app name of `default` and two criteria named `default`, one serial and one
/// parallel. The names of the functions are `plato_create_criterion` and `plato_create_parallel_criterion`
/// respectively.
services::AppConfigurationWithDirectory test_app_configuration(const std::filesystem::path& aSharedLibPath);
}  // namespace plato::integration_tests::utilities

#endif
