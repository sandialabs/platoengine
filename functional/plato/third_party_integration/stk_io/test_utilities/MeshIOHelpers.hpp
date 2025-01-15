#ifndef PLATO_THIRD_PARTY_INTEGRATION_STK_IO_TEST_UTILITIES_MESHIOHELPERS
#define PLATO_THIRD_PARTY_INTEGRATION_STK_IO_TEST_UTILITIES_MESHIOHELPERS

#include <filesystem>
#include <functional>
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

/// @brief A function for retrieving a scalar field value using an index into some data structure.
using ScalarFieldFunction = std::function<double(std::size_t)>;

/// @brief Given a pathname  @a aInputMeshName, create a new mesh on disk @a aOutputMeshName that has an additional
/// nodal field stored in the name and populated with the data in @a aScalarField.
void write_nodal_scalar_field(const std::filesystem::path& aInputMeshName,
                              const ScalarFieldFunction& aScalarField,
                              const std::string_view aFieldName,
                              const std::filesystem::path& aOutputMeshName);

/// @brief Given a pathname  @a aInputMeshName, create a new mesh on disk @a aOutputMeshName that has an additional
/// element field stored in the name and populated with the data in @a aScalarField.
void write_element_scalar_field(const std::filesystem::path& aInputMeshName,
                                const ScalarFieldFunction& aScalarField,
                                const std::string_view aFieldName,
                                const std::filesystem::path& aOutputMeshName);

}  // namespace plato::third_party_integration::stk_io::test_utilities

#endif
