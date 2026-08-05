#ifndef PLATO_THIRD_PARTY_INTEGRATION_KRINO_BACKGROUNDMESH
#define PLATO_THIRD_PARTY_INTEGRATION_KRINO_BACKGROUNDMESH

#include <Akri_MeshInterface.hpp>
#include <Akri_Phase_Support.hpp>
#include <filesystem>
#include <memory>
#include <set>
#include <string>
#include <vector>

#include "plato/third_party_integration/krino/LevelSetPrimitives.hpp"

namespace plato::third_party_integration::krino
{
/// @brief a struct for holding both the krino representation of mesh data and the level set fields associated with that
/// mesh.
struct BackgroundMesh
{
    std::unique_ptr<::krino::MeshInterface> mKrinoMesh;
    std::vector<::krino::LS_Field> mLevelSetFields;
};

/// @brief sets up the appropriate level set fields and activates the krino mesh from a mesh on disk at @a aFileName.
/// Returns a BackgroundMesh containing the mesh meta data and level-set fields with the default value of 0.
///
/// Excludes blocks with names given in @a aExcludedBlocks. Typically, @a aExcludedBlocks will be the fixed (non-design)
/// blocks.
///
/// @pre The environment for krino was initialized.
/// @pre @a aFileName is a valid exodus mesh file on disk.
auto background_mesh_with_zero_level_set_field(const std::filesystem::path& aFileName,
                                               const std::set<std::string>& aExcludedBlocks = {}) -> BackgroundMesh;

/// @brief sets up the appropriate level set fields and activates the krino mesh from a mesh on disk at @a aFileName.
/// Returns a BackgroundMesh containing the mesh meta data and level-set fields with values set as the signed-distances
/// from the primitives in @a aLevelSetPrimitives.
///
/// Excludes blocks with names given in @a aExcludedBlocks. Typically, @a aExcludedBlocks will be the fixed (non-design)
/// blocks.
///
/// @pre The environment for krino was initialized.
/// @pre @a aFileName is a valid exodus mesh file on disk.
auto background_mesh_from_primitives(const std::filesystem::path& aFileName,
                                     const LevelSetPrimitives& aLevelSetPrimitives,
                                     const std::set<std::string>& aExcludedBlocks = {}) -> BackgroundMesh;

}  // namespace plato::third_party_integration::krino

#endif
