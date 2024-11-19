#ifndef PLATO_THIRD_PARTY_INTEGRATION_KRINO_UTILITIES
#define PLATO_THIRD_PARTY_INTEGRATION_KRINO_UTILITIES

#include <mpi.h>

#include <filesystem>
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

void setup_fields_for_conforming_decomposition(const stk::mesh::MetaData &aMeta);

void create_bounding_box_mesh(const stk::math::Vector3d &aMinCorner,
                              const stk::math::Vector3d &aMaxCorner,
                              const double aMeshSize,
                              const std::filesystem::path &aFilename);

}  // namespace plato::third_party_integration::krino

#endif
