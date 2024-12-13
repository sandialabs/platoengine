#ifndef PLATO_THIRDPARTYINTEGRATION_KRINO_INTERFACE
#define PLATO_THIRDPARTYINTEGRATION_KRINO_INTERFACE

#include <filesystem>
#include <unordered_map>

#include "plato/third_party_integration/krino/LevelSetPrimitives.hpp"
#include "plato/third_party_integration/krino/Utilities.hpp"
#include "plato/utilities/NamedType.hpp"

namespace plato::analysis
{
struct AnalysisDomainMesh;
}

namespace plato::third_party_integration::krino
{

using BackgroundMeshFilePath = utilities::NamedType<std::filesystem::path, struct BackgroundMeshNameTag>;
using CutMeshFilePath = utilities::NamedType<std::filesystem::path, struct BackgroundMeshNameTag>;

using LevelSetJacobian = std::unordered_map<stk::mesh::EntityId, LevelSetJacobianColumn>;

/// @brief Generates a computational mesh, @a aCutMesh, by cutting a background mesh.
/// @param aBackgroundMeshWithLevelSets AnalysisDomainMesh whose scalar field contains the level set values.
/// @param aCutMesh File path at which to create the cut mesh.
/// @param aIncludeVoidRegion specifies whether to include the void region defined by the level set values in the
/// computational mesh.
/// @return The sensitivities of the computational mesh's boundary nodes with respect to the level set values.
auto generate_computational_mesh(const analysis::AnalysisDomainMesh &aBackgroundMeshWithLevelSets,
                                 const CutMeshFilePath &aCutMesh,
                                 const VoidPhase aVoidRegion) -> LevelSetJacobian;

/// @brief Generate a computational mesh, @a aCutMesh, by cutting a background mesh,
/// @a aBackgroundMeshName, with the given level set primitives, @a aLevelSetPrimitives.  Returns the
/// level set values on the background mesh resulting from the level set primitives.
/// @a aIncludeVoidRegion specifies whether to include the void region defined by the
/// level set values in the computational mesh.
[[nodiscard]] auto initialize_mesh_with_level_set_primitives(const BackgroundMeshFilePath &aBackgroundMeshName,
                                                             const CutMeshFilePath &aCutMesh,
                                                             const LevelSetPrimitives &aLevelSetPrimitives,
                                                             const VoidPhase aVoidRegion) -> std::vector<double>;

/// @brief Given a vector @a aRowVector, the Jacobian of the level-set mapping @a aLevelSetJacobian (sensitivity of
/// nodal coordinates to level set values), and the local-to-global node id map of the background mesh, @a
/// aBackgroundMeshSpaceIDs, computes the product of the row vector with the Jacobian of the level-set mapping,
/// representing the chain rule of the gradient of the objective with respect to the level set values.
[[nodiscard]] auto level_set_row_vector_jacobian_product(const std::vector<double> &aRowVector,
                                                         const analysis::AnalysisDomainMesh &aCutMeshSpaceIDs,
                                                         const LevelSetJacobian &aLevelSetJacobian,
                                                         analysis::AnalysisDomainMesh &&aBackgroundMeshSpaceIDs)
    -> analysis::AnalysisDomainMesh;

/// @brief Computes the product of a row vector (represented by @a aBackgroundLevelSetSpaceVector) and adjoint Jacobian
/// matrix (represented by @a aLevelSetJacobian).
///
/// The result is stored in a map, which maps a cut mesh global node ID to a 3-vector.
[[nodiscard]] auto level_set_row_vector_adjoint_jacobian_product(
    const analysis::AnalysisDomainMesh &aBackgroundLevelSetSpaceVector, const LevelSetJacobian &aLevelSetJacobian)
    -> std::unordered_map<KrinoGlobalNodeID, stk::math::Vector3d>;

}  // namespace plato::third_party_integration::krino

#endif
