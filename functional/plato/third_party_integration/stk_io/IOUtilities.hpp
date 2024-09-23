#ifndef PLATO_THIRDPARTYINTEGRATION_STKIO_IOUTILITIES
#define PLATO_THIRDPARTYINTEGRATION_STKIO_IOUTILITIES

#include <filesystem>
#include <functional>
#include <memory>
#include <stk_mesh/base/Types.hpp>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "plato/third_party_integration/stk_io/CommandGenerator.hpp"

namespace stk::mesh
{
// In trilinos 15.1, there are conflicting forward declarations of BulkData, of which clang-tidy disapproves.
class BulkData;  // NOLINT
}  // namespace stk::mesh

namespace plato::third_party_integration::stk_io
{
/// @brief Given a pathname  @a aMeshName and the Command generator @a aCommandGenerator, write to disk the data in
/// exodus format
void write_mesh(const std::filesystem::path& aMeshName, const CommandGenerator& aCommandGenerator);

/// @brief Given a pathname  @a aMeshName and mesh description (e.g. `textmesh`), write to disk the data in
/// exodus format
void write_mesh(const std::filesystem::path& aMeshName, std::string_view aMeshDescription);

/// @brief Use a STK @a aGenerationCommand, e.g., "generated:1x1x1" to create and return a shared pointer to a STK Bulk
[[nodiscard]] std::shared_ptr<stk::mesh::BulkData> generate_bulk_data(const CommandGenerator& aCommandGenerator);

/// @brief Given a pathname  @a aMeshName and the STK Bulk data @a aBulk, write to disk the data in exodus format
void write_bulk_data(const std::filesystem::path& aMeshName, std::shared_ptr<stk::mesh::BulkData> aBulk);

/// @brief Given a pathname  @a aMeshName, read from disk and return a shared pointer to the STK Bulk data.
[[nodiscard]] std::shared_ptr<stk::mesh::BulkData> read_mesh_bulk_data(const std::filesystem::path& aMeshName);

/// @brief A function for retrieving a scalar field value using an index into some data structure.
using ScalarFieldFunction = std::function<double(std::size_t)>;

/// @brief Given a pathname  @a aInputMeshName, create a new mesh on disk @a aOutputMeshName that has an additional
/// nodal field stored in the name and populated with the data in @a aScalarField.
/// @pre aDensity.size() == size<stk::topology::NODE_RANK>()
void write_nodal_scalar_field(const std::filesystem::path& aInputMeshName,
                              const ScalarFieldFunction& aScalarField,
                              const std::string_view aFieldName,
                              const std::filesystem::path& aOutputMeshName);

/// @brief Given a pathname  @a aInputMeshName, create a new mesh on disk @a aOutputMeshName that has an additional
/// element field stored in the name and populated with the data in @a aScalarField.
/// @pre aDensity.size() == size<stk::topology::ELEMENT_RANK>()
void write_element_scalar_field(const std::filesystem::path& aInputMeshName,
                                const ScalarFieldFunction& aScalarField,
                                const std::string_view aFieldName,
                                const std::filesystem::path& aOutputMeshName);

}  // namespace plato::third_party_integration::stk_io

#endif
