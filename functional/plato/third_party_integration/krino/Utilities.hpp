#ifndef PLATO_THIRD_PARTY_INTEGRATION_KRINO_UTILITIES
#define PLATO_THIRD_PARTY_INTEGRATION_KRINO_UTILITIES

#include <mpi.h>

#include <map>
#include <stk_math/StkVector.hpp>
#include <stk_mesh/base/Types.hpp>
#include <utility>
#include <vector>

#include "plato/third_party_integration/common/Vector3.hpp"

using namespace plato::third_party_integration::common;

namespace plato::third_party_integration::krino
{

using KrinoGlobalNodeID = unsigned int;

enum struct DFDXFormatting
{
    GlobalID,
    OneToN
};

enum Dimension
{
    X = 0,
    Y,
    Z
};

struct InterfaceNodeDXDP
{
    std::vector<stk::mesh::EntityId> mParentNodeIds;
    std::vector<stk::math::Vector3d> mParentDXDP;
};

struct SphereLocatorData
{
    std::vector<std::pair<double, double>> mStartAndSpacing;
    SphereLocatorData(const int &aSize, const double &aInitialValue)
        : mStartAndSpacing(aSize, std::pair(aInitialValue, aInitialValue))
    {
    }
};

struct SpherePatternData
{
    bool mSpheresCanOverlapBoundingBox = false;
    std::vector<double> mBoundingBoxMinXYZ;
    std::vector<double> mBoundingBoxMaxXYZ;
    std::vector<int> mNumSpheres;
    double mSphereRadius = 0.0;
};

struct Plane
{
    Vector3 mNormal{0.0, 0.0, 0.0};
    double mOffset = 0.0;
};

struct Sphere
{
    Coordinate mCenter{0.0, 0.0, 0.0};
    double mRadius = 0.0;
};

struct LevelsetPrimitives
{
    std::vector<Plane> mPlanes;
    std::vector<Sphere> mSpheres;
};

/// @brief Initialization needed for krino to run correctly.
void initialize_environment_for_krino(const MPI_Comm &aComm);

/// @brief Given sphere pattern input, @a aData, generate sphere primitives to be used to initialize a krino levelset.
[[nodiscard]] auto generate_spheres(const SpherePatternData &aData) -> std::vector<Sphere>;

/// @brief Given sphere pattern input, @a aData, calculate the pattern start coordinates in x, y, and z, and their
/// spacing in x, y, and z.
[[nodiscard]] auto calculate_sphere_starts_and_spacing(const SpherePatternData &aData) -> SphereLocatorData;

/// @brief Validate sphere pattern data @a aData.
[[nodiscard]] auto calculate_overlapping_single_sphere_locator_data(const SpherePatternData &aPatternData,
                                                                    const size_t &aDimension)
    -> std::pair<double, double>;

/// @brief Validate sphere pattern data @a aData.
[[nodiscard]] auto calculate_overlapping_many_sphere_locator_data(const SpherePatternData &aPatternData,
                                                                  const size_t &aDimension)
    -> std::pair<double, double>;

/// @brief Validate sphere pattern data @a aData.
[[nodiscard]] auto calculate_non_overlapping_sphere_locator_data(const SpherePatternData &aPatternData,
                                                                 const size_t &aDimension) -> std::pair<double, double>;

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
