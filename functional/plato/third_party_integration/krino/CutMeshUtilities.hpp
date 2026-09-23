#ifndef PLATO_THIRD_PARTY_INTEGRATION_KRINO_CUTMESHUTILITIES
#define PLATO_THIRD_PARTY_INTEGRATION_KRINO_CUTMESHUTILITIES

#include <Akri_MeshInterface.hpp>
#include <cstdint>
#include <filesystem>

#include "plato/third_party_integration/krino/SnappingParameters.hpp"

namespace plato::third_party_integration::krino
{
struct BackgroundMesh;

/// @brief Describes whether or not to include a void phase block in the generated cut mesh.
enum struct VoidPhase : std::uint8_t
{
    kIncludeInMesh,
    kExcludeFromMesh
};

/// @brief Take a background mesh @a aBackgroundMesh and perform the cutting of the mesh and creation of child nodes.
/// Each child node generated in the cut operation have only two parents and those parents are background nodes. The
/// reverse procedure for cut_mesh is:
///    ::krino::CDMesh::reset_mesh_to_original_undecomposed_state(aBackgroundMesh.bulkData());
/// @pre the environment for krino was initialized by calling 'set_stk_communicator_for_krino', the level set fields in
/// the krino mesh were set up by constructing a background mesh, and the level-set field values were set appropriately
void cut_mesh(const BackgroundMesh& aBackgroundMesh, SnappingParameters aSnappingParameters);

/// @brief Takes a KrinoMesh @a aKrinoMesh and a VoidPhase specification @a aVoidPhase and determines the nodes in the
/// cut mesh. This includes the new nodes on the cut mesh surface as well as the background nodes. Background nodes that
/// would be eliminated from a cut_mesh operation are NOT included.
/// @pre the environment for krino was initialized by calling 'set_stk_communicator_for_krino', the level set fields in
/// the krino mesh were set up by constructing a background mesh, the level-set field values were set appropriately, and
/// 'cut_mesh' was called.
[[nodiscard]] auto cut_mesh_node_ids(const ::krino::MeshInterface& aKrinoMesh,
                                     VoidPhase aVoidPhase) -> std::vector<stk::mesh::EntityId>;

/// @brief Given a modifiable bulk data @a aBulkData and specified void phase @a aVoidPhase, create a valid
/// stk::mesh::Selector for the cut mesh. Also apply the Krino fixes for errors in parallel writing a cut mesh.
[[nodiscard]] auto create_output_selector(stk::mesh::BulkData& aBulkData, VoidPhase aVoidPhase) -> stk::mesh::Selector;

/// @brief Take a krino mesh's bulk data @a aBulkData and use it to write the cut mesh to the file specified in @a
/// aOutputFileName. The void phase @a aVoidPhase specifies whether to include the void region or not when writing to
/// disk.
/// @pre the environment for krino was initialized by calling 'set_stk_communicator_for_krino', the level set fields in
/// the krino mesh were set up by constructing a background mesh, the level-set field values were set appropriately, and
/// 'cut_mesh' was called.
void write_cut_mesh(const stk::mesh::BulkData& aBulkData,
                    const std::filesystem::path& aOutputFileName,
                    const stk::mesh::Selector& aSelector);

}  // namespace plato::third_party_integration::krino

#endif
