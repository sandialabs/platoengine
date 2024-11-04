#ifndef PLATO_UTILITIES_DATAFILEPATH
#define PLATO_UTILITIES_DATAFILEPATH

#include <filesystem>
#include <optional>
#include <string_view>

namespace plato::utilities
{

/// @brief Retrieves the location of the test_data based on CMAKE variables.
///
/// CMAKE variables FUNCTIONAL_TEST_DATA_BUILD_PATH, FUNCTIONAL_TEST_DATA_INSTALL_PATH give the location on disk for the
/// file given in @a aFileName in the folder `data`
std::optional<std::filesystem::path> data_file_path(const std::string_view aFileName);

}  // namespace plato::utilities

#endif
