#ifndef PLATO_THIRD_PARTY_INTEGRATION_KRINO_INTERFACE
#define PLATO_THIRD_PARTY_INTEGRATION_KRINO_INTERFACE

#include <filesystem>
#include <unordered_map>

#include "plato/third_party_integration/krino/LevelsetPrimitives.hpp"
#include "plato/third_party_integration/krino/Utilities.hpp"
#include "plato/utilities/NamedType.hpp"

namespace plato::third_party_integration::krino
{

using BackgroundMeshFilePath = utilities::NamedType<std::filesystem::path, struct BackgroundMeshNameTag>;
using CutMeshFilePath = utilities::NamedType<std::filesystem::path, struct BackgroundMeshNameTag>;

/// @brief Generate a computational mesh, @a aCutMesh, by cutting a background mesh,
/// @a aBackgroundMeshName, with the given levelset values, @a aLevelsetValues.  Returns the
/// sensitivities of the computational mesh's boundary nodes with respect to the levelset values.
/// @a aIncludeVoidRegion specifies whether to include the void region defined by the
/// levelset values in the computational mesh.
auto generate_computational_mesh(const BackgroundMeshFilePath &aBackgroundMeshName,
                                 const CutMeshFilePath &aCutMesh,
                                 const std::vector<double> &aLevelsetValues,
                                 const bool aIncludeVoidRegion)
    -> std::unordered_map<stk::mesh::EntityId, InterfaceNodeDXDP>;

/// @brief Generate a computational mesh, @a aCutMesh, by cutting a background mesh,
/// @a aBackgroundMeshName, with the given levelset primitives, @a aLevelsetPrimitives.  Returns the
/// levelset values on the background mesh resulting from the levelset primitives.
/// @a aIncludeVoidRegion specifies whether to include the void region defined by the
/// levelset values in the computational mesh.
[[nodiscard]] auto initialize_mesh_with_levelset_primitives(const BackgroundMeshFilePath &aBackgroundMeshName,
                                                            const CutMeshFilePath &aCutMesh,
                                                            const LevelsetPrimitives &aLevelsetPrimitives,
                                                            const bool aIncludeVoidRegion) -> std::vector<double>;

/// @brief Given DFDX, @a aDFDXMap (sensitivity of objective to nodal coordinate changes), and DXDP, @a aDXDP
/// (sensitivity of nodal coordinates to levelset values), and the local-to-global node id map of the background mesh,
/// @a aBackgroundMeshSpaceIDs, perform the chain rule to get DFDLS (sensitivity of objective to levelset values).
[[nodiscard]] auto calculate_dfdls(const std::vector<double> &aDFDX,
                                   const analysis::AnalysisDomainMesh &aCutMeshSpaceIDs,
                                   const std::unordered_map<stk::mesh::EntityId, InterfaceNodeDXDP> &aDXDP,
                                   analysis::AnalysisDomainMesh &&aBackgroundMeshSpaceIDs)
    -> analysis::AnalysisDomainMesh;

/// @brief Computes the product of a row vector (represented by @a aBackgroundLevelSetSpaceVector) and adjoint Jacobian
/// matrix (represented by @a aDXDP).
///
/// The result is stored in a map, which maps a cut mesh global node ID to a 3-vector.
[[nodiscard]] auto calculate_adjoint_dfdls(const analysis::AnalysisDomainMesh &aBackgroundLevelSetSpaceVector,
                                           const std::unordered_map<stk::mesh::EntityId, InterfaceNodeDXDP> &aDXDP)
    -> std::unordered_map<KrinoGlobalNodeID, stk::math::Vector3d>;

}  // namespace plato::third_party_integration::krino

#endif  // PLATO_THIRD_PARTY_INTEGRATION_KRINO_INTERFACE
