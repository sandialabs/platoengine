#ifndef PLATO_MESH_MESHBLOCKS
#define PLATO_MESH_MESHBLOCKS

#include <optional>
#include <string_view>

#include "plato/mesh/Mesh.hpp"
#include "plato/third_party_integration/common/BlockData.hpp"

namespace plato::mesh
{
/// @brief A mixin class extending Mesh and providing operations on mesh blocks.
struct MeshBlocks : public Mesh
{
    using BlockIDType = third_party_integration::common::BlockData::BlockIDType;

    MeshBlocks(Mesh aMeshBase);

    /// @brief Returns the block id for the block with name @a aBlockName if it exists, an empty optional otherwise
    [[nodiscard]] auto blockID(std::string_view aBlockName) const -> std::optional<BlockIDType>;

    /// @brief Returns the names and ids of all blocks in the mesh.
    /// @post The elements of the vector are sorted in ascending order based on the block id.
    [[nodiscard]] auto blockData() const -> std::vector<third_party_integration::common::BlockData>;

    /// @brief Returns the node IDs associated with the block with name @a aBlockName if it exists, or an empty vector
    /// if it does not exist.
    [[nodiscard]] auto nodeIDs(std::string_view aBlockName) const -> std::vector<std::size_t>;

    /// @brief Returns the element IDs associated with the block with name @a aBlockName if it exists, or an empty
    /// vector if it does not exist.
    [[nodiscard]] auto elementIDs(std::string_view aBlockName) const -> std::vector<std::size_t>;
};

}  // namespace plato::mesh

#endif
