#ifndef PLATO_THIRDPARTYINTEGRATION_STKIO_UTILITIES
#define PLATO_THIRDPARTYINTEGRATION_STKIO_UTILITIES

#include <filesystem>
#include <functional>
#include <memory>
#include <optional>
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

/// @brief Given a STK Bulk data  @a aBulk, return the total number of nodes.
[[nodiscard]] unsigned int node_size(const stk::mesh::BulkData& aBulk);

/// @brief Given a STK Bulk data  @a aBulk, return the total number of elements.
[[nodiscard]] unsigned int element_size(const stk::mesh::BulkData& aBulk);

/// @brief Given a STK Bulk data  @a aBulk, return the spatial dimensions of the mesh.
[[nodiscard]] unsigned int spatial_dimensions(const stk::mesh::BulkData& aBulk);

/// @brief Given a STK Bulk data  @a aBulk, return a std::vector of the nodal coordinates ordered x0,y0,z0,x1,y1,z1, ...
/// For 2D, only x and y coordinates are included in the vector.
[[nodiscard]] std::vector<double> flattened_nodal_coordinates(const stk::mesh::BulkData& aBulk);

/// @brief Given a STK Bulk data  @a aBulk, return a std::vector of Coordinates
/// For 2D, z values of Coordinates are set to 0.
[[nodiscard]] auto nodal_coordinates(const stk::mesh::BulkData& aBulk) -> std::vector<common::Coordinate>;

/// @brief Returns the list of element IDs associated with the block @a aPart in mesh @a aBulkData.
[[nodiscard]] auto nodal_coordinates(const stk::mesh::BulkData& aBulk, const PartReferenceVector& aParts)
    -> std::vector<common::Coordinate>;

/// @brief Given a pathname  @a aMeshName, return a std::vector of the nodal densities stored in the kTopologyField name
[[nodiscard]] std::vector<double> read_nodal_density(const std::filesystem::path& aMeshName);

/// @brief Given a pathname  @a aMeshName, return a std::vector of the element densities stored in the kTopologyField
/// name
[[nodiscard]] std::vector<double> read_element_density(const std::filesystem::path& aMeshName);

/// @brief Given a pathname  @a aInputMeshName, create a new mesh on disk @a aOutputMeshName that has an additional
/// nodal field stored in the kTopologyField name and populated with the data in @a aDensity
/// @pre aDensity.size() == size<stk::topology::NODE_RANK>()
void write_nodal_density(const std::filesystem::path& aInputMeshName,
                         const std::vector<double>& aDensity,
                         const std::filesystem::path& aOutputMeshName);

/// @brief Given a pathname  @a aInputMeshName, create a new mesh on disk @a aOutputMeshName that has an additional
/// element field stored in the kTopologyField name and populated with the data in @a aDensity
/// @pre aDensity.size() == size<stk::topology::ELEMENT_RANK>()
void write_element_density(const std::filesystem::path& aInputMeshName,
                           const std::vector<double>& aDensity,
                           const std::filesystem::path& aOutputMeshName);

/// @brief Given a STK Bulk data  @a aBulk, return the STK element container
stk::mesh::EntityVector element_vector(const stk::mesh::BulkData& aBulk);

/// @brief Given a STK bulk data @a aBulk and part @a aPart, returns the STK element container.
stk::mesh::EntityVector element_vector(const stk::mesh::BulkData& aBulk, const stk::mesh::Part& aPart);

/// @brief Given a STK bulk data @a aBulk and parts @a aParts, returns the STK element container.
stk::mesh::EntityVector element_vector(const stk::mesh::BulkData& aBulk, const PartReferenceVector& aParts);

}  // namespace plato::third_party_integration::stk_io

#endif
