#ifndef PLATO_THIRD_PARTY_INTEGRATION_KRINO_UTILITIES
#define PLATO_THIRD_PARTY_INTEGRATION_KRINO_UTILITIES

#include <mpi.h>

#include <stk_math/StkVector.hpp>
#include <stk_mesh/base/Types.hpp>
#include <unordered_map>
#include <utility>
#include <vector>

namespace Plato
{
struct InterfaceNodeDXDP
{
    std::vector<stk::mesh::EntityId> mParentNodeIds;
    std::vector<stk::math::Vector3d> mParentDXDP;
};

/// @brief Initialization needed for krino to run correctly.
void initialize_environment_for_krino(const MPI_Comm &aComm);

}  // namespace Plato

#endif
