#ifndef PLATO_THIRD_PARTY_INTEGRATION_KRINO_UTILITIES
#define PLATO_THIRD_PARTY_INTEGRATION_KRINO_UTILITIES

#include <mpi.h>

#include <map>
#include <stk_math/StkVector.hpp>
#include <stk_mesh/base/Types.hpp>
#include <utility>
#include <vector>

namespace plato::third_party_integration::krino
{

using KrinoGlobalNodeID = unsigned int;

enum struct DFDXFormatting
{
    GlobalID,
    OneToN
};

struct InterfaceNodeDXDP
{
    std::vector<stk::mesh::EntityId> mParentNodeIds;
    std::vector<stk::math::Vector3d> mParentDXDP;
};

/// @brief Initialization needed for krino to run correctly.
void initialize_environment_for_krino(const MPI_Comm &aComm);

/// @brief Given DFDX values on the computational mesh, @a aDFDX, the computational mesh local-to-global node id map, @a
/// aCutMeshGlobalNodeIDMap, and the type of formatting for DFDX, @a aDFDXFormatting, generate and return a map from
/// global node ID to DFDX values.
[[nodiscard]] auto assemble_global_id_to_dfdx_map(const std::vector<double> &aDFDX,
                                                  const std::vector<KrinoGlobalNodeID> &aCutMeshGlobalNodeIDMap,
                                                  const DFDXFormatting aDFDXFormatting)
    -> std::unordered_map<KrinoGlobalNodeID, stk::math::Vector3d>;

/// @brief Given DFDX, @a aDFDXMap (sensitivity of objective to nodal coordinate changes), and DXDP, @a aDXDP
/// (sensitivity of nodal coordinates to levelset values), and the local-to-global node id map of the backgroun mesh, @a
/// aBackgroundNodemap, perform the chain rule to get DFDLS (sensitivity of objective to levelset values).
[[nodiscard]] auto calculate_dfdls(const std::unordered_map<KrinoGlobalNodeID, stk::math::Vector3d> &aDFDXMap,
                                   const std::unordered_map<stk::mesh::EntityId, InterfaceNodeDXDP> &aDXDP,
                                   const std::vector<KrinoGlobalNodeID> &aBackgroundNodemap)
    -> std::unordered_map<KrinoGlobalNodeID, double>;

}  // namespace plato::third_party_integration::krino

#endif  // PLATO_THIRD_PARTY_INTEGRATION_KRINO_UTILITIES
