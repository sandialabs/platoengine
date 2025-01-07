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

    explicit MeshBlocks(Mesh aMeshBase);

    /// @brief Returns the block ordinal for the block with name @a aBlockName if it exists, an empty optional
    /// otherwise.
    /// @note The block ordinal is an internally generated id, and not the same as the block ID in the mesh.
    [[nodiscard]] auto blockOrdinal(std::string_view aBlockName) const -> std::optional<BlockOrdinalType>;

    /// @brief Returns the block id for the block with name @a aBlockName if it exists, an empty optional otherwise
    [[nodiscard]] auto blockID(std::string_view aBlockName) const -> std::optional<BlockIDType>;

    /// @brief Returns the block id for the block with ordinal @a aBlockOrdinal if it exists, an empty optional
    /// otherwise.
    /// @note A block ordinal is an internally assigned ID, different from the one in the original mesh.
    [[nodiscard]] auto blockID(BlockOrdinalType aBlockOrdinal) const -> std::optional<BlockIDType>;

    /// @brief Returns the names and ids of all blocks in the mesh.
    /// @post The elements of the vector are sorted in ascending order based on the block id.
    [[nodiscard]] auto blockData() const -> std::vector<third_party_integration::common::BlockData>;

    /// @brief Returns the node IDs associated with the block with ordinal @a aBlockOrdinal if it exists, or an empty
    /// vector if it does not exist.
    /// @note A block ordinal is an internally assigned ID, different from the one in the original mesh.
    [[nodiscard]] auto nodeIDs(BlockOrdinalType aBlockOrdinal) const -> std::vector<std::size_t>;

    /// @brief Returns the element IDs associated with the block with ordinal @a aBlockOrdinal if it exists, or an empty
    /// vector if it does not exist.
    /// @note A block ordinal is an internally assigned ID, different from the one in the original mesh.
    [[nodiscard]] auto elementIDs(BlockOrdinalType aBlockOrdinal) const -> std::vector<std::size_t>;

    /// @brief Returns all the block names found in the mesh, which is mainly useful for error messages.
    [[nodiscard]] auto blockNames() const -> std::vector<std::string>;
};

/// @brief Converts all ordinals @a aBlockOrdinals (which are used by stk) to block IDs, which are used by
/// AnalysisDomainMesh.
/// @pre All ordinals in @a aBlockOrdinals are valid block ordinals in @a aMesh. Checked by an assertion.
auto block_ids(const Mesh& aMesh, const std::vector<Mesh::BlockOrdinalType>& aBlockOrdinals)
    -> std::vector<MeshBlocks::BlockIDType>;

}  // namespace plato::mesh

#endif
