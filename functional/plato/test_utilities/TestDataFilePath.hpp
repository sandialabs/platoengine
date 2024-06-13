#ifndef PLATO_TEST_UTILITIES_TESTDATAFILEPATH
#define PLATO_TEST_UTILITIES_TESTDATAFILEPATH

#include <filesystem>
#include <optional>
#include <string_view>

namespace plato::test_utilities
{

/// @brief Retrieves the location of the test_data based on CMAKE variables.
///
/// CMAKE variables FUNCTIONAL_TEST_DATA_BUILD_PATH, FUNCTIONAL_TEST_DATA_INSTALL_PATH give the location on disk for the
/// file given in @a aFileName in the folder test_utilities/data
std::optional<std::filesystem::path> test_data_file_path(const std::string_view aFileName);

}  // namespace plato::test_utilities

#endif
