#ifndef PLATO_MESH_MESH
#define PLATO_MESH_MESH

#include <filesystem>
#include <memory>
#include <optional>
#include <vector>

#include "plato/third_party_integration/common/BlockData.hpp"
#include "plato/third_party_integration/common/Vector3.hpp"

namespace stk::mesh
{
// In trilinos 15.1, there are conflicting forward declarations of BulkData, of which clang-tidy disapproves.
class BulkData;  // NOLINT
}  // namespace stk::mesh

namespace plato::mesh
{

/// @brief Provides common mesh operations such as retrieving the number of elements or nodal coordinates.
class Mesh
{
   public:
    Mesh(const std::filesystem::path& aMeshName);

    /// @brief The total number of elements in the mesh
    [[nodiscard]] unsigned int numberOfElements() const;
    /// @brief The total number of nodes in the mesh
    [[nodiscard]] unsigned int numberOfNodes() const;
    /// @brief The total number of blocks in the mesh
    [[nodiscard]] unsigned int numberOfBlocks() const;
    /// @brief Returns the block id for the block with name @a aBlockName if it exists, an empty optional otherwise
    [[nodiscard]] std::optional<unsigned int> blockId(std::string_view aBlockName) const;
    /// @brief Returns the names and ids of all blocks in the mesh.
    [[nodiscard]] auto blockData() const -> std::vector<third_party_integration::common::BlockData>;
    /// @brief The dimensions of the mesh (2 or 3).
    [[nodiscard]] unsigned int spatialDimensions() const;
    /// @brief The nodal coordinates ordered as x0,y0,z0,x1,y1,z1
    /// @note For 2D, only x and y coordinates are included in the vector.
    [[nodiscard]] std::vector<double> flattenedNodalCoordinates() const;
    /// @brief All nodal coordinates in the mesh
    [[nodiscard]] std::vector<third_party_integration::common::Coordinate> nodalCoordinates() const;
    /// @brief Centroids of all the elements in the mesh.
    [[nodiscard]] std::vector<third_party_integration::common::Coordinate> elementCentroids() const;
    /// @brief The total volume of the mesh.
    [[nodiscard]] double volume() const;

   private:
    std::shared_ptr<stk::mesh::BulkData> mBulk;
};

}  // namespace plato::mesh

#endif
