#ifndef PLATO_THIRDPARTYINTEGRATION_STKIO_UTILITIES
#define PLATO_THIRDPARTYINTEGRATION_STKIO_UTILITIES

#include <filesystem>
#include <functional>
#include <memory>
#include <stk_mesh/base/Types.hpp>
#include <string_view>
#include <vector>

#include "plato/third_party_integration/stk_io/CommandGenerator.hpp"

namespace stk::mesh
{
// In trilinos 15.1, there are conflicting forward declarations of BulkData, of which clang-tidy disapproves.
class BulkData;  // NOLINT
class Part;
}  // namespace stk::mesh

namespace plato::third_party_integration::stk_io
{
using PartReferenceVector = std::vector<std::reference_wrapper<const stk::mesh::Part>>;

/// @brief Given a STK Bulk data  @a aBulk, return the total number of nodes.
[[nodiscard]] unsigned int node_size(const stk::mesh::BulkData& aBulk);

/// @brief Given a STK Bulk data  @a aBulk, return the total number of nodes only in the parts list @a aParts.
[[nodiscard]] unsigned int node_size(const stk::mesh::BulkData& aBulk, const PartReferenceVector& aParts);

/// @brief Given a STK Bulk data  @a aBulk, return the total number of elements.
[[nodiscard]] unsigned int element_size(const stk::mesh::BulkData& aBulk);

/// @brief Given a STK Bulk data  @a aBulk, return the total number of elements only in the parts list @a aParts.
[[nodiscard]] unsigned int element_size(const stk::mesh::BulkData& aBulk, const PartReferenceVector& aParts);

/// @brief Given a STK Bulk data  @a aBulk, return the spatial dimensions of the mesh.
[[nodiscard]] unsigned int spatial_dimensions(const stk::mesh::BulkData& aBulk);

/// @brief Given a STK Bulk data  @a aBulk, return a std::vector of Coordinates
/// For 2D, z values of Coordinates are set to 0.
[[nodiscard]] auto nodal_coordinates(const stk::mesh::BulkData& aBulk) -> std::vector<common::Coordinate>;

/// @brief Returns the list of element IDs associated with the block @a aPart in mesh @a aBulkData.
[[nodiscard]] auto nodal_coordinates(const stk::mesh::BulkData& aBulk, const PartReferenceVector& aParts)
    -> std::vector<common::Coordinate>;

/// @brief Given a pathname  @a aFilename, return a std::vector of the global node id map
/// name
[[nodiscard]] std::vector<unsigned int> extract_global_node_ids(const std::string& aFilename);

/// @brief Given a STK Bulk data  @a aBulk, return the STK element container
stk::mesh::EntityVector element_vector(const stk::mesh::BulkData& aBulk);

/// @brief Given a STK bulk data @a aBulk and part @a aPart, returns the STK element container.
stk::mesh::EntityVector element_vector(const stk::mesh::BulkData& aBulk, const stk::mesh::Part& aPart);

/// @brief Given a STK bulk data @a aBulk and parts @a aParts, returns the STK element container.
stk::mesh::EntityVector element_vector(const stk::mesh::BulkData& aBulk, const PartReferenceVector& aParts);

}  // namespace plato::third_party_integration::stk_io

#endif
