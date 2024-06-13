#include <mpi.h>

#include <stk_math/StkVector.hpp>

#include "Plato_InputData.hpp"

#pragma once

namespace Plato::Krino
{

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
LevelsetPrimitives readLevelsetInitializationData(Plato::InputData &aAppFileData);
std::vector<Sphere> generateSpheres(const SpherePatternData &aData);
void createSpheresFromPattern(const Plato::InputData &aNode, LevelsetPrimitives &aPrims);
void createSphere(const Plato::InputData &aNode, LevelsetPrimitives &aPrims);
void createPlane(const Plato::InputData &aNode, LevelsetPrimitives &aPrims);
SpherePatternData readSpherePatternData(const Plato::InputData &aNode);
Sphere readSphereData(const Plato::InputData &aNode);
Plane readPlaneData(const Plato::InputData &aNode);
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

}  // namespace Plato::Krino
