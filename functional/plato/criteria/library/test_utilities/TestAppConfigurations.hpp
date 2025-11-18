#ifndef PLATO_CRITERIA_LIBRARY_TEST_UTILITIES_TESTAPPCONFIGURATION
#define PLATO_CRITERIA_LIBRARY_TEST_UTILITIES_TESTAPPCONFIGURATION

#include "plato/test_utilities/TestDirectorySetupTeardown.hpp"

namespace plato::criteria::library::test_utilities
{
/// @brief Creates a directory and writes an app configuration file.
/// @return A TestDirectorySetupTeardown object managing the test directory with the app configuration, and the name of
/// the app.
[[nodiscard]] auto test_configurations()
    -> std::pair<plato::test_utilities::TestDirectorySetupTeardown, std::string_view>;

}  // namespace plato::criteria::library::test_utilities

#endif
