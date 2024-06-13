#ifndef PLATO_UTILITIES_FILEUTILITIES
#define PLATO_UTILITIES_FILEUTILITIES

#include <filesystem>

namespace plato::utilities
{
/// @brief Creates a unique filename based on @a aBasePath.
///
/// The parent path will be identical, but the file name will have a unique hexadecimal
/// number appended before the file extension. The unique number is randomly generated
/// and will be 16 hexadecimal digits.
/// @note If @a aBasePath does not contain a filename (as specified by `std::filesystem::path::has_filename`)
/// then the path is returned without change.
std::filesystem::path make_filename_unique(std::filesystem::path aBasePath);
}  // namespace plato::utilities

#endif
