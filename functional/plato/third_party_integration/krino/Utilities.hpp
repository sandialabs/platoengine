#ifndef PLATO_THIRD_PARTY_INTEGRATION_KRINO_UTILITIES
#define PLATO_THIRD_PARTY_INTEGRATION_KRINO_UTILITIES

#include <mpi.h>

#include <stk_math/StkVector.hpp>
#include <stk_mesh/base/Types.hpp>
#include <unordered_map>
#include <utility>
#include <vector>

namespace plato::analysis
{
struct AnalysisDomainMesh;
}

namespace plato::third_party_integration::krino
{

using KrinoGlobalNodeID = unsigned int;

struct InterfaceNodeDXDP
{
    std::vector<stk::mesh::EntityId> mParentNodeIds;
    std::vector<stk::math::Vector3d> mParentDXDP;
};

/// @brief Initialization needed for krino to run correctly.
void initialize_environment_for_krino(const MPI_Comm &aComm);

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

#endif
