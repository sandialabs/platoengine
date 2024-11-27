#ifndef PLATO_THIRDPARTYINTEGRATION_KRINO_INTERFACE
#define PLATO_THIRDPARTYINTEGRATION_KRINO_INTERFACE

#include <filesystem>
#include <unordered_map>

#include "plato/third_party_integration/krino/LevelSetPrimitives.hpp"
#include "plato/third_party_integration/krino/Utilities.hpp"
#include "plato/utilities/NamedType.hpp"

namespace plato::third_party_integration::krino
{

using BackgroundMeshFilePath = utilities::NamedType<std::filesystem::path, struct BackgroundMeshNameTag>;
using CutMeshFilePath = utilities::NamedType<std::filesystem::path, struct BackgroundMeshNameTag>;

using LevelSetJacobian = std::unordered_map<stk::mesh::EntityId, LevelSetJacobianColumn>;

/// @brief Generate a computational mesh, @a aCutMesh, by cutting a background mesh,
/// @a aBackgroundMeshName, with the given level set values, @a aLevelSetValues.  Returns the
/// sensitivities of the computational mesh's boundary nodes with respect to the level set values.
/// @a aIncludeVoidRegion specifies whether to include the void region defined by the
/// level set values in the computational mesh.
auto generate_computational_mesh(const BackgroundMeshFilePath &aBackgroundMeshName,
                                 const CutMeshFilePath &aCutMesh,
                                 const std::vector<double> &aLevelSetValues,
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

/// @brief Given DFDX, @a aDFDXMap (sensitivity of objective to nodal coordinate changes), and the Jacobian of the
/// level-set mapping, @a aLevelSetJacobian (sensitivity of nodal coordinates to level set values), and the
/// local-to-global node id map of the background mesh, @a aBackgroundMeshSpaceIDs, perform the chain rule to get DFDLS
/// (sensitivity of objective to level set values).
[[nodiscard]] auto calculate_dfdls(const std::vector<double> &aDFDX,
                                   const analysis::AnalysisDomainMesh &aCutMeshSpaceIDs,
                                   const LevelSetJacobian &aLevelSetJacobian,
                                   analysis::AnalysisDomainMesh &&aBackgroundMeshSpaceIDs)
    -> analysis::AnalysisDomainMesh;

/// @brief Computes the product of a row vector (represented by @a aBackgroundLevelSetSpaceVector) and adjoint Jacobian
/// matrix (represented by @a aLevelSetJacobian).
///
/// The result is stored in a map, which maps a cut mesh global node ID to a 3-vector.
[[nodiscard]] auto calculate_adjoint_dfdls(const analysis::AnalysisDomainMesh &aBackgroundLevelSetSpaceVector,
                                           const LevelSetJacobian &aLevelSetJacobian)
    -> std::unordered_map<KrinoGlobalNodeID, stk::math::Vector3d>;

}  // namespace plato::third_party_integration::krino

#endif
