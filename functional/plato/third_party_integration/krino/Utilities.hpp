#ifndef PLATO_THIRDPARTYINTEGRATION_KRINO_UTILITIES
#define PLATO_THIRDPARTYINTEGRATION_KRINO_UTILITIES

#include <mpi.h>

#include <filesystem>
#include <stk_math/StkVector.hpp>
#include <stk_mesh/base/Types.hpp>
#include <vector>

namespace plato::analysis
{
struct AnalysisDomainMesh;
}

namespace plato::third_party_integration::krino
{
/// @brief Describes whether or not to include a void phase block in the generated cut mesh.
enum struct VoidPhase
{
    kIncludeInMesh,
    kExcludeFromMesh
};

using KrinoGlobalNodeID = unsigned int;
using BoundingBox = std::pair<stk::math::Vector3d, stk::math::Vector3d>;

struct InterfaceNodeDXDP
{
    std::vector<stk::mesh::EntityId> mParentNodeIds;
    std::vector<stk::math::Vector3d> mParentDXDP;
};

/// @brief Initialization needed for krino to run correctly.
void initialize_environment_for_krino(const std::filesystem::path &aLogFile, const MPI_Comm &aComm);

void create_bounding_box_mesh(const BoundingBox &aBoundingBox,
                              const double aMeshSize,
                              const std::filesystem::path &aFilename);

}  // namespace plato::third_party_integration::krino

#endif
