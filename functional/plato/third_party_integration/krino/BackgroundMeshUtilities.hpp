#ifndef PLATO_THIRD_PARTY_INTEGRATION_KRINO_BACKGROUNDMESHUTILITIES
#define PLATO_THIRD_PARTY_INTEGRATION_KRINO_BACKGROUNDMESHUTILITIES

#include <stk_mesh/base/Types.hpp>

namespace plato::third_party_integration::krino
{
struct BackgroundMesh;

/// @brief Takes a BackgroundMesh @a aBackgroundMesh and determines the background node ids in the cut mesh.
/// @pre the environment for krino was initialized by calling 'set_stk_communicator_for_krino', the level set fields in
/// the krino mesh were set up by constructing a background mesh, the level-set field values were set appropriately, and
/// 'cut_mesh' was called.
[[nodiscard]] auto background_node_ids(const BackgroundMesh& aBackgroundMesh) -> std::vector<stk::mesh::EntityId>;

/// @brief Given a background mesh @a aBackgroundMesh transform the values of its level set field into an unordered
/// map of global id to double.
[[nodiscard]] auto get_level_set_values(const BackgroundMesh& aBackgroundMesh)
    -> std::unordered_map<stk::mesh::EntityId, double>;

/// @brief Take a background mesh @a aBackgroundMesh and return the background nodes after a cut operation.
/// @pre the environment for krino was initialized by calling 'set_stk_communicator_for_krino', the level set fields in
/// the krino mesh were set up by constructing a background mesh, and the level-set field values were set appropriately
[[nodiscard]] auto node_entities_in_mesh(const BackgroundMesh& aBackgroundMesh) -> stk::mesh::EntityVector;
}  // namespace plato::third_party_integration::krino

#endif
