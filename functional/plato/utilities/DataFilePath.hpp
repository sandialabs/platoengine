#ifndef PLATO_UTILITIES_DATAFILEPATH
#define PLATO_UTILITIES_DATAFILEPATH

#include <filesystem>
#include <optional>

namespace plato::utilities
{

/// @brief Retrieves the location of the test_data based on CMAKE variables.
///
/// CMAKE variables FUNCTIONAL_TEST_DATA_BUILD_PATH, FUNCTIONAL_TEST_DATA_INSTALL_PATH give the location on disk for the
/// file given in @a aFileName in the folder `data`
auto data_file_path(const std::filesystem::path& aFileName) -> std::optional<std::filesystem::path>;

}  // namespace plato::utilities

#endif
