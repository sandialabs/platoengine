#ifndef PLATO_THIRD_PARTY_INTEGRATION_KRINO_BLOCKUTILITIES
#define PLATO_THIRD_PARTY_INTEGRATION_KRINO_BLOCKUTILITIES

#include <Akri_MeshInterface.hpp>
#include <set>
#include <stk_mesh/base/Types.hpp>
#include <string>

namespace plato::third_party_integration::krino
{
/// @brief Returns the blocks contained in @a aKrinoMesh as a PartVector, excluding any irrelevant, Krino-added blocks.
[[nodiscard]] auto all_block_parts(const ::krino::MeshInterface& aKrinoMesh) -> stk::mesh::PartVector;

/// @brief Returns the blocks contained in @a aKrinoMesh as a PartVector, excluding any irrelevant, Krino-added blocks
/// and all blocks with names in @a aExcludedBlocks.
[[nodiscard]] auto all_blocks_except(const ::krino::MeshInterface& aKrinoMesh,
                                     const std::set<std::string>& aExcludedBlocks) -> stk::mesh::PartVector;

}  // namespace plato::third_party_integration::krino

#endif
