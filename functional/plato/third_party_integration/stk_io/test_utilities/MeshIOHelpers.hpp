#ifndef PLATO_THIRD_PARTY_INTEGRATION_STK_IO_TEST_UTILITIES_MESHIOHELPERS
#define PLATO_THIRD_PARTY_INTEGRATION_STK_IO_TEST_UTILITIES_MESHIOHELPERS

#include <filesystem>
#include <string_view>
#include <vector>

namespace plato::third_party_integration::stk_io::test_utilities
{
/// @brief Reads the field with name @a aFieldName from the mesh on disk at path @a aMeshPath
[[nodiscard]] auto read_nodal_field_as_vector(const std::filesystem::path& aMeshPath, std::string_view aFieldName)
    -> std::vector<double>;

/// @brief Reads the field with name @a aFieldName from the mesh on disk at path @a aMeshPath
[[nodiscard]] auto read_element_field_as_vector(const std::filesystem::path& aMeshPath, std::string_view aFieldName)
    -> std::vector<double>;

}  // namespace plato::third_party_integration::stk_io::test_utilities

#endif
