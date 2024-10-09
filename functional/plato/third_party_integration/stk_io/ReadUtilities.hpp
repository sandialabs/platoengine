#ifndef PLATO_THIRDPARTYINTEGRATION_STKIO_READUTILITIES
#define PLATO_THIRDPARTYINTEGRATION_STKIO_READUTILITIES

#include <filesystem>
#include <functional>
#include <map>
#include <memory>
#include <stk_mesh/base/Types.hpp>
#include <string_view>
#include <vector>

#include "plato/third_party_integration/common/Vector3.hpp"

namespace stk::mesh
{
// In trilinos 15.1, there are conflicting forward declarations of BulkData, of which clang-tidy disapproves.
class BulkData;  // NOLINT
class Part;
}  // namespace stk::mesh

namespace plato::third_party_integration::stk_io
{
using PartReferenceVector = std::vector<std::reference_wrapper<const stk::mesh::Part>>;

/// @brief Given a pathname  @a aMeshName, read from disk and return a shared pointer to the STK Bulk data.
[[nodiscard]] std::shared_ptr<stk::mesh::BulkData> read_mesh_bulk_data(const std::filesystem::path& aMeshName);

/// @brief Given a STK Bulk data  @a aBulk, return the total number of nodes.
[[nodiscard]] unsigned int node_size(const stk::mesh::BulkData& aBulk);

/// @brief Given a STK Bulk data  @a aBulk, return the total number of nodes only in the parts list @a aParts.
[[nodiscard]] unsigned int node_size(const stk::mesh::BulkData& aBulk, const PartReferenceVector& aParts);

/// @brief Given a STK Bulk data  @a aBulk, return the total number of elements.
[[nodiscard]] unsigned int element_size(const stk::mesh::BulkData& aBulk);

/// @brief Given a STK Bulk data @a aBulk, return the total number of elements only in the parts list @a aParts.
[[nodiscard]] unsigned int element_size(const stk::mesh::BulkData& aBulk, const PartReferenceVector& aParts);

/// @brief Given a STK Bulk data @a aBulk, return the spatial dimensions of the mesh.
[[nodiscard]] unsigned int spatial_dimensions(const stk::mesh::BulkData& aBulk);

/// @brief Given a STK Bulk data @a aBulk, return a std::vector of Coordinates
/// For 2D, z values of Coordinates are set to 0.
[[nodiscard]] auto nodal_coordinates(const stk::mesh::BulkData& aBulk) -> std::vector<common::Coordinate>;

/// @brief Returns nodal coordinates associated with the block @a aPart in mesh @a aBulkData.
[[nodiscard]] auto nodal_coordinates(const stk::mesh::BulkData& aBulk, const PartReferenceVector& aParts)
    -> std::vector<common::Coordinate>;

/// @brief Returns nodal IDs associated with the parts in @a aParts in mesh @a aBulkData.
[[nodiscard]] auto node_ids(const stk::mesh::BulkData& aBulk, const PartReferenceVector& aParts)
    -> std::vector<std::size_t>;

/// @brief Returns nodal IDs associated with the block @a aPart in mesh @a aBulkData.
[[nodiscard]] auto node_ids(const stk::mesh::BulkData& aBulk, const stk::mesh::Part& aPart) -> std::vector<std::size_t>;

/// @brief Returns element IDs associated with the block @a aPart in mesh @a aBulkData.
[[nodiscard]] auto element_ids(const stk::mesh::BulkData& aBulk, const stk::mesh::Part& aPart)
    -> std::vector<std::size_t>;

/// @brief Given a STK Bulk data @a aBulk, return the STK element container
stk::mesh::EntityVector element_vector(const stk::mesh::BulkData& aBulk);

/// @brief Given a STK bulk data @a aBulk and part @a aPart, returns the STK element container.
stk::mesh::EntityVector element_vector(const stk::mesh::BulkData& aBulk, const stk::mesh::Part& aPart);

/// @brief Given a STK bulk data @a aBulk and parts @a aParts, returns the STK element container.
stk::mesh::EntityVector element_vector(const stk::mesh::BulkData& aBulk, const PartReferenceVector& aParts);

/// @brief Given a pathname @a aInputMeshName, check if the field @a aFieldName exists on the elements
[[nodiscard]] bool element_field_exists(const std::filesystem::path& aInputMeshName, const std::string_view aFieldName);

/// @brief Given a pathname @a aInputMeshName, check if the field @a aFieldName exists on the nodes
[[nodiscard]] bool nodal_field_exists(const std::filesystem::path& aInputMeshName, const std::string_view aFieldName);

/// @brief Given a pathname @a aInputMeshName, return a list of all the field names defined on the nodes
[[nodiscard]] auto nodal_field_names(const std::filesystem::path& aInputMeshName) -> std::vector<std::string>;

/// @brief Given a pathname @a aInputMeshName, read the field @a aFieldName
[[nodiscard]] auto read_element_field(const std::filesystem::path& aInputMeshName, const std::string_view aFieldName)
    -> std::map<std::size_t, double>;

/// @brief Given a pathname @a aInputMeshName, read the field @a aFieldName
/// @pre The nodal field exists on the mesh otherwise an empty map is returned
[[nodiscard]] auto read_nodal_field(const std::filesystem::path& aInputMeshName, const std::string_view aFieldName)
    -> std::map<std::size_t, double>;

/// @brief Given a pathname  @a aFilename, return a std::vector of the global node id map
/// name
[[nodiscard]] std::vector<unsigned int> extract_global_node_ids(const stk::mesh::BulkData& aBulkData);

}  // namespace plato::third_party_integration::stk_io

#endif
