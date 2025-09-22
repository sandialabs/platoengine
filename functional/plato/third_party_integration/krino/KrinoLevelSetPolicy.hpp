#ifndef PLATO_THIRD_PARTY_INTEGRATION_KRINO_LEVELSETINITIALIZATION
#define PLATO_THIRD_PARTY_INTEGRATION_KRINO_LEVELSETINITIALIZATION

#include <set>
#include <stk_mesh/base/Types.hpp>
#include <string>

/// @file This code is mainly lifted from internal functions in krino/krino/krino_lib/Akri_LevelSetPolicy.cpp. It is not
/// API-accessible and so it is reproduced here. This code should be moved to krino.

namespace krino
{
class MeshInterface;
}

namespace plato::third_party_integration::krino
{
/// @brief Sets up the level-set fields on all blocks except those with names in @a aExcludedBlocks with a void phase
/// for any negative level-set.
void setup_level_sets(::krino::MeshInterface& aKrinoMesh, const std::set<std::string>& aExcludedBlocks);

/// @brief Returns the blocks contained in @a aKrinoMesh as a PartVector, excluding any irrelevant, Krino-added blocks.
[[nodiscard]] auto all_block_parts(const ::krino::MeshInterface& aKrinoMesh) -> stk::mesh::PartVector;

/// @brief Returns the blocks contained in @a aKrinoMesh as a PartVector, excluding any irrelevant, Krino-added blocks
/// and all blocks with names in @a aExcludedBlocks.
[[nodiscard]] auto all_blocks_except(const ::krino::MeshInterface& aKrinoMesh,
                                     const std::set<std::string>& aExcludedBlocks) -> stk::mesh::PartVector;

}  // namespace plato::third_party_integration::krino

#endif
