#ifndef PLATO_THIRDPARTYINTEGRATION_STKIO_READUTILITIES
#define PLATO_THIRDPARTYINTEGRATION_STKIO_READUTILITIES

#include <filesystem>
#include <functional>
#include <map>
#include <memory>
#include <stk_mesh/base/Types.hpp>
#include <string_view>
#include <vector>

#include "plato/third_party_integration/common/BlockData.hpp"
#include "plato/third_party_integration/common/BoundingBox.hpp"
#include "plato/third_party_integration/common/Vector3.hpp"
#include "plato/utilities/ValueOrTag.hpp"

namespace stk::mesh
{
// In trilinos 15.1, there are conflicting forward declarations of BulkData, of which clang-tidy disapproves.
class BulkData;  // NOLINT
class Part;
}  // namespace stk::mesh

namespace plato::third_party_integration::stk_io
{
using PartReferenceVector = std::vector<std::reference_wrapper<const stk::mesh::Part>>;

/// @brief A type tag used for specifying reading of the last time step in a file.
struct LastTimeStep
{
};

using TimeStep = utilities::ValueOrTag<double, LastTimeStep>;

/// @brief Given a pathname  @a aMeshName, read from disk and return a shared pointer to the STK Bulk data.
[[nodiscard]] auto read_mesh_bulk_data(const std::filesystem::path& aMeshName) -> std::shared_ptr<stk::mesh::BulkData>;

/// @brief Given a STK Bulk data  @a aBulk, return the total number of nodes.
[[nodiscard]] auto node_size(const stk::mesh::BulkData& aBulk) -> unsigned int;

/// @brief Given a STK Bulk data  @a aBulk, return the total number of nodes only in the parts list @a aParts.
[[nodiscard]] auto node_size(const stk::mesh::BulkData& aBulk, const PartReferenceVector& aParts) -> unsigned int;

/// @brief Given a STK Bulk data  @a aBulk, return the total number of elements.
[[nodiscard]] auto element_size(const stk::mesh::BulkData& aBulk) -> unsigned int;

/// @brief Given a STK Bulk data @a aBulk, return the total number of elements only in the parts list @a aParts.
[[nodiscard]] auto element_size(const stk::mesh::BulkData& aBulk, const PartReferenceVector& aParts) -> unsigned int;

/// @brief Given a STK Bulk data @a aBulk, return the spatial dimensions of the mesh.
[[nodiscard]] auto spatial_dimensions(const stk::mesh::BulkData& aBulk) -> unsigned int;

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
auto element_vector(const stk::mesh::BulkData& aBulk) -> stk::mesh::EntityVector;

/// @brief Given a STK bulk data @a aBulk and part @a aPart, returns the STK element container.
auto element_vector(const stk::mesh::BulkData& aBulk, const stk::mesh::Part& aPart) -> stk::mesh::EntityVector;

/// @brief Given a STK bulk data @a aBulk and parts @a aParts, returns the STK element container.
auto element_vector(const stk::mesh::BulkData& aBulk, const PartReferenceVector& aParts) -> stk::mesh::EntityVector;

/// @brief Given a pathname @a aInputMeshName, check if the field @a aFieldName exists on the elements
[[nodiscard]] auto element_field_exists(const std::filesystem::path& aInputMeshName, std::string_view aFieldName)
    -> bool;

/// @brief Given a pathname @a aInputMeshName, check if the field @a aFieldName exists on the nodes
[[nodiscard]] auto nodal_field_exists(const std::filesystem::path& aInputMeshName, std::string_view aFieldName) -> bool;

/// @brief Given a pathname @a aInputMeshName, return a list of all the field names defined on the nodes
[[nodiscard]] auto nodal_field_names(const std::filesystem::path& aInputMeshName) -> std::vector<std::string>;

/// @brief Given a pathname @a aInputMeshName, read the field @a aFieldName at time step @a aTime
/// @pre The element field exists on the mesh otherwise an empty map is returned
[[nodiscard]] auto read_element_field(const std::filesystem::path& aInputMeshName,
                                      std::string_view aFieldName,
                                      TimeStep aTime = LastTimeStep{}) -> std::map<std::size_t, double>;

/// @brief Given a pathname @a aInputMeshName, read the field @a aFieldName at time step @a aTime
/// @pre The nodal field exists on the mesh otherwise an empty map is returned
[[nodiscard]] auto read_nodal_field(const std::filesystem::path& aInputMeshName,
                                    std::string_view aFieldName,
                                    TimeStep aTime = LastTimeStep{}) -> std::map<std::size_t, double>;

/// @brief Returns the time steps associated with the data in the mesh @a aInputMeshName.
[[nodiscard]] auto time_steps(const std::filesystem::path& aInputMeshName) -> std::vector<double>;

/// @brief Returns the bounding box of the part @a aPart associated with BulkData @a aBulkData.
[[nodiscard]] auto bounding_box(const stk::mesh::BulkData& aBulkData, const PartReferenceVector& aParts)
    -> common::BoundingBox;

/// @brief Returns the part associated with the entire @a aBulk data.
[[nodiscard]] auto universal_part(const stk::mesh::BulkData& aBulk) -> PartReferenceVector;

/// @brief Returns true if all the elements in the bulk data @a aBulk are tet or tri types.
[[nodiscard]] auto all_tet_or_tri_elements(const stk::mesh::BulkData& aBulk) -> bool;

/// @brief Returns true if the part @a aPart has a topology type of tet or tri.
[[nodiscard]] auto all_tet_or_tri_elements(const stk::mesh::Part& aPart) -> bool;

namespace detail
{
/// @brief Returns the mesh selector for the given PartReferenceVector @a aParts.
[[nodiscard]] auto parts_to_selector(const PartReferenceVector& aParts) -> stk::mesh::Selector;
}  // namespace detail

}  // namespace plato::third_party_integration::stk_io

#endif
