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

struct InterfaceNode_DXDP
{
    std::vector<stk::mesh::EntityId> parentNodeIds;
    std::vector<stk::math::Vector3d> parentDXDP;
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
    std::vector<double> mCoordMins;
    std::vector<double> mCoordMaxes;
    std::vector<int> mNumSpheres;
    double mSphereRadius = 0.0;
};

struct Plane
{
    double mNormalX = 0.0;
    double mNormalY = 0.0;
    double mNormalZ = 0.0;
    double mOffset = 0.0;
};

struct Sphere
{
    double mCenterX = 0.0;
    double mCenterY = 0.0;
    double mCenterZ = 0.0;
    double mRadius = 0.0;
};

struct LevelsetPrimitives
{
    std::vector<Plane> mPlanes;
    std::vector<Sphere> mSpheres;
};

enum struct DFDXFormatting
{
    GlobalID,
    OneToN
};

void initializeKrinoLogging();
void initializeSTKEnvironment(const MPI_Comm &aComm);
std::vector<Sphere> generateSpheres(const SpherePatternData &aData);
SphereLocatorData calculateSphereStartsAndSpacing(const SpherePatternData &aData);
void checkForReasonableSpherePatternDefinition(const SpherePatternData &aData);
std::pair<double, double> calculateOverlappingSingleSphereLocatorData(const SpherePatternData &aPatternData,
                                                                      const size_t &aDimension);
std::pair<double, double> calculateOverlappingManySphereLocatorData(const SpherePatternData &aPatternData,
                                                                    const size_t &aDimension);
std::pair<double, double> calculateNonOverlappingSphereLocatorData(const SpherePatternData &aPatternData,
                                                                   const size_t &aDimension);
std::map<unsigned int, stk::math::Vector3d> assembleGlobalIDToDFDXMap(
    const std::vector<double> &aDFDX,
    const std::vector<double> &aCutMeshGlobalNodeIDMap,
    const DFDXFormatting aDFDXFormatting);
std::map<unsigned int, stk::math::Vector3d> assembleGlobalIDToDFDXMap(
    const std::vector<double> &aDFDX,
    const std::vector<unsigned int> &aCutMeshGlobalNodeIDMap,
    const DFDXFormatting aDFDXFormatting);
std::map<unsigned int, double> calculateDFDLS(const std::map<unsigned int, stk::math::Vector3d> &aDFDXMap,
                                              const std::map<stk::mesh::EntityId, InterfaceNode_DXDP> &aDXDP,
                                              const std::vector<unsigned int> &aBackgroundNodemap);

}  // namespace plato::third_party_integration::krino

#endif  // PLATO_THIRD_PARTY_INTEGRATION_KRINO_UTILITIES
