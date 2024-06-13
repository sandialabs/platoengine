#include "PlatoKrinoUtilities.hpp"

#include <Akri_DiagWriter.hpp>
#include <Plato_Exceptions.hpp>
#include <stk_util/diag/WriterRegistry.hpp>
#include <stk_util/environment/Env.hpp>
#include <stk_util/environment/EnvData.hpp>
#include <stk_util/environment/OutputLog.hpp>

namespace Plato::Krino
{

template <typename T>
T getOrThrow(const Plato::InputData &aInputData, const std::string &aName, const std::string &aError)
{
    T tReturnValue;
    if (aInputData.size<std::string>(aName) == 0)
    {
        throw std::runtime_error(aError.c_str());
    }
    if constexpr (std::is_same_v<T, bool>)
    {
        tReturnValue = Plato::Get::Bool(aInputData, aName);
    }
    else if constexpr (std::is_same_v<T, int>)
    {
        tReturnValue = Plato::Get::Int(aInputData, aName);
    }
    else if constexpr (std::is_same_v<T, double>)
    {
        tReturnValue = Plato::Get::Double(aInputData, aName);
    }
    else
    {
        throw std::runtime_error(aError.c_str());
    }
    return tReturnValue;
}

void initializeSTKEnvironment(const MPI_Comm &aComm)
{
    stk::EnvData::instance().m_parallelComm = aComm;
    MPI_Comm_size(stk::EnvData::parallel_comm(), &stk::EnvData::instance().m_parallelSize);
    MPI_Comm_rank(stk::EnvData::parallel_comm(), &stk::EnvData::instance().m_parallelRank);
}

void initializeKrinoLogging()
{
    sierra::Diag::registerWriter("krinolog", krinolog, krino::theDiagWriterParser());
    const std::string output_description = "out>pout dout>out";
    std::string parallel_output_description = " pout>null";
    stk::bind_output_streams(
        output_description +
        parallel_output_description);  // necessary for krinolog to work, otherwise you may get segfault
}

void checkForReasonableSpherePatternDefinition(const SpherePatternData &aData)
{
    for (size_t i = 0; i < aData.mCoordMins.size(); ++i)
    {
        if (aData.mCoordMins[i] >= aData.mCoordMaxes[i])
        {
            throw std::runtime_error("ERROR: Ill-defined bounding box for sphere pattern.");
        }
        if (aData.mNumSpheres[i] < 1)
        {
            throw std::runtime_error("ERROR: There must be at least on sphere in each direction in a sphere pattern.");
        }
    }
    if (aData.mSphereRadius <= 0.0)
    {
        throw std::runtime_error("ERROR: Sphere radius must be greater than 0.0 when defining a sphere pattern.");
    }
}

SpherePatternData readSpherePatternData(const Plato::InputData &aNode)
{
    SpherePatternData tData;
    tData.mCoordMins = {
        getOrThrow<double>(aNode, "bbox_xmin", "ERROR: Levelset definition bounding box xmin was not specified."),
        getOrThrow<double>(aNode, "bbox_ymin", "ERROR: Levelset definition bounding box ymin was not specified."),
        getOrThrow<double>(aNode, "bbox_zmin", "ERROR: Levelset definition bounding box zmin was not specified.")};
    tData.mCoordMaxes = {
        getOrThrow<double>(aNode, "bbox_xmax", "ERROR: Levelset definition bounding box xmax was not specified."),
        getOrThrow<double>(aNode, "bbox_ymax", "ERROR: Levelset definition bounding box ymax was not specified."),
        getOrThrow<double>(aNode, "bbox_zmax", "ERROR: Levelset definition bounding box zmax was not specified.")};
    tData.mNumSpheres = {
        getOrThrow<int>(aNode, "num_spheres_x",
                        "ERROR: Levelset definition number of spheres in the x direction was not specified."),
        getOrThrow<int>(aNode, "num_spheres_y",
                        "ERROR: Levelset definition number of spheres in the y direction was not specified."),
        getOrThrow<int>(aNode, "num_spheres_z",
                        "ERROR: Levelset definition number of spheres in the z direction was not specified.")};
    tData.mSphereRadius =
        getOrThrow<double>(aNode, "radius", "ERROR: Levelset definition sphere radius was not specified.");
    tData.mSpheresCanOverlapBoundingBox =
        getOrThrow<bool>(aNode, "overlap_bbox", "ERROR: Levelset definition overlap param not specified.");
    return tData;
}

Sphere readSphereData(const Plato::InputData &aNode)
{
    double tCenterX =
        getOrThrow<double>(aNode, "center_x", "ERROR: Levelset definition sphere center_x was not specified.");
    double tCenterY =
        getOrThrow<double>(aNode, "center_y", "ERROR: Levelset definition sphere center_y was not specified.");
    double tCenterZ =
        getOrThrow<double>(aNode, "center_z", "ERROR: Levelset definition sphere center_z was not specified.");
    double tRadius = getOrThrow<double>(aNode, "radius", "ERROR: Levelset definition sphere radius was not specified.");
    return Sphere{tCenterX, tCenterY, tCenterZ, tRadius};
}

Plane readPlaneData(const Plato::InputData &aNode)
{
    double tNormalX =
        getOrThrow<double>(aNode, "normal_x", "ERROR: Levelset definition plane normal_x was not specified.");
    double tNormalY =
        getOrThrow<double>(aNode, "normal_y", "ERROR: Levelset definition plane normal_y was not specified.");
    double tNormalZ =
        getOrThrow<double>(aNode, "normal_z", "ERROR: Levelset definition plane normal_z was not specified.");
    double tOffset = getOrThrow<double>(aNode, "offset", "ERROR: Levelset definition plane offset was not specified.");
    return Plane{tNormalX, tNormalY, tNormalZ, tOffset};
}

void createSpheresFromPattern(const Plato::InputData &aNode, LevelsetPrimitives &aPrims)
{
    SpherePatternData tData = readSpherePatternData(aNode);
    std::vector<Sphere> tSpheres = generateSpheres(tData);
    aPrims.mSpheres.insert(aPrims.mSpheres.end(), tSpheres.begin(), tSpheres.end());
    ;
}

void createSphere(const Plato::InputData &aNode, LevelsetPrimitives &aPrims)
{
    Sphere tSphere = readSphereData(aNode);
    aPrims.mSpheres.push_back(tSphere);
}

void createPlane(const Plato::InputData &aNode, LevelsetPrimitives &aPrims)
{
    Plane tPlane = readPlaneData(aNode);
    aPrims.mPlanes.push_back(tPlane);
}

LevelsetPrimitives readLevelsetInitializationData(Plato::InputData &aAppFileData)
{
    LevelsetPrimitives tPrims;
    if (!aAppFileData.empty())
    {
        for (auto tNode : aAppFileData.getByName<Plato::InputData>("Operation"))
        {
            std::string tStrName = Plato::Get::String(tNode, "Name");
            if (tStrName == "Initialize Levelsets")
            {
                auto tSpherePatternNodes = tNode.getByName<Plato::InputData>("SpherePattern");
                for (auto tCurSpherePatternNode : tSpherePatternNodes)
                {
                    createSpheresFromPattern(tCurSpherePatternNode, tPrims);
                }
                auto tSphereNodes = tNode.getByName<Plato::InputData>("Sphere");
                for (auto tCurSphereNode : tSphereNodes)
                {
                    createSphere(tCurSphereNode, tPrims);
                }
                auto tPlaneNodes = tNode.getByName<Plato::InputData>("Plane");
                for (auto tCurPlaneNode : tPlaneNodes)
                {
                    createPlane(tCurPlaneNode, tPrims);
                }
            }
        }
    }
    return tPrims;
}

std::pair<double, double> calculateOverlappingSingleSphereLocatorData(const SpherePatternData &aPatternData,
                                                                      const size_t &aDimension)
{
    std::pair<double, double> tStartAndSpacing(
        aPatternData.mCoordMins[aDimension],
        (aPatternData.mCoordMaxes[aDimension] - aPatternData.mCoordMins[aDimension]) / 2.0);
    return tStartAndSpacing;
}

std::pair<double, double> calculateOverlappingManySphereLocatorData(const SpherePatternData &aPatternData,
                                                                    const size_t &aDimension)
{
    const double tSpacing = (aPatternData.mCoordMaxes[aDimension] - aPatternData.mCoordMins[aDimension]) /
                            (aPatternData.mNumSpheres[aDimension] - 1);
    std::pair<double, double> tStartAndSpacing(aPatternData.mCoordMins[aDimension] - tSpacing, tSpacing);
    return tStartAndSpacing;
}

std::pair<double, double> calculateNonOverlappingSphereLocatorData(const SpherePatternData &aPatternData,
                                                                   const size_t &aDimension)
{
    std::pair<double, double> tStartAndSpacing(
        aPatternData.mCoordMins[aDimension],
        (aPatternData.mCoordMaxes[aDimension] - aPatternData.mCoordMins[aDimension]) /
            (aPatternData.mNumSpheres[aDimension] + 1));
    return tStartAndSpacing;
}

SphereLocatorData calculateSphereStartsAndSpacing(const SpherePatternData &aData)
{
    constexpr int tNumDimensions = 3;
    SphereLocatorData tLocatorData(tNumDimensions, 0.0);
    for (size_t tCurDimension = 0; tCurDimension < tNumDimensions; ++tCurDimension)
    {
        if (aData.mSpheresCanOverlapBoundingBox)
        {
            if (aData.mNumSpheres[tCurDimension] == 1)
            {
                tLocatorData.mStartAndSpacing[tCurDimension] =
                    calculateOverlappingSingleSphereLocatorData(aData, tCurDimension);
            }
            else
            {
                tLocatorData.mStartAndSpacing[tCurDimension] =
                    calculateOverlappingManySphereLocatorData(aData, tCurDimension);
            }
        }
        else
        {
            tLocatorData.mStartAndSpacing[tCurDimension] =
                calculateNonOverlappingSphereLocatorData(aData, tCurDimension);
        }
    }
    return tLocatorData;
}

std::vector<Sphere> generateSpheres(const SpherePatternData &aData)
{
    checkForReasonableSpherePatternDefinition(aData);

    SphereLocatorData tLocatorData = calculateSphereStartsAndSpacing(aData);

    // Loop to create 3D array of spheres
    std::vector<Sphere> tSpheres;
    for (int i = 0; i < aData.mNumSpheres[0]; ++i)
    {
        const double tSphereCenterX =
            tLocatorData.mStartAndSpacing[0].first + (i + 1) * tLocatorData.mStartAndSpacing[0].second;
        for (int j = 0; j < aData.mNumSpheres[1]; ++j)
        {
            const double tSphereCenterY =
                tLocatorData.mStartAndSpacing[1].first + (j + 1) * tLocatorData.mStartAndSpacing[1].second;
            for (int k = 0; k < aData.mNumSpheres[2]; ++k)
            {
                const double tSphereCenterZ =
                    tLocatorData.mStartAndSpacing[2].first + (k + 1) * tLocatorData.mStartAndSpacing[2].second;
                tSpheres.push_back(Sphere{tSphereCenterX, tSphereCenterY, tSphereCenterZ, aData.mSphereRadius});
            }
        }
    }
    return tSpheres;
}

std::map<unsigned int, stk::math::Vector3d> assembleGlobalIDToDFDXMap(
    const std::vector<double> &aDFDX,
    const std::vector<double> &aCutMeshGlobalNodeIDMap,
    const DFDXFormatting aDFDXFormatting)
{
    unsigned int tNumNodes = aCutMeshGlobalNodeIDMap.size();

    std::map<unsigned int, stk::math::Vector3d> tGlobalIDToDFDXMap;
    for (unsigned int i = 0; i < tNumNodes; ++i)
    {
        unsigned int tCurGlobalNodeID = aCutMeshGlobalNodeIDMap[i];
        unsigned int tDFDXIndex = 0;
        if (aDFDXFormatting == DFDXFormatting::GlobalID)
        {
            tDFDXIndex = 3 * (tCurGlobalNodeID - 1);
        }
        else if (aDFDXFormatting == DFDXFormatting::OneToN)
        {
            tDFDXIndex = 3 * i;
        }
        else
        {
            throw std::runtime_error("ERROR: Unrecognized formatting for DFDX.");
        }
        tGlobalIDToDFDXMap[tCurGlobalNodeID] = {aDFDX[tDFDXIndex], aDFDX[tDFDXIndex + 1], aDFDX[tDFDXIndex + 2]};
    }
    return tGlobalIDToDFDXMap;
}

std::map<unsigned int, stk::math::Vector3d> assembleGlobalIDToDFDXMap(
    const std::vector<double> &aDFDX,
    const std::vector<unsigned int> &aCutMeshGlobalNodeIDMap,
    const DFDXFormatting aDFDXFormatting)
{
    unsigned int tNumNodes = aCutMeshGlobalNodeIDMap.size();

    std::map<unsigned int, stk::math::Vector3d> tGlobalIDToDFDXMap;
    for (unsigned int i = 0; i < tNumNodes; ++i)
    {
        unsigned int tCurGlobalNodeID = aCutMeshGlobalNodeIDMap[i];
        unsigned int tDFDXIndex = 0;
        if (aDFDXFormatting == DFDXFormatting::GlobalID)
        {
            tDFDXIndex = 3 * (tCurGlobalNodeID - 1);
        }
        else if (aDFDXFormatting == DFDXFormatting::OneToN)
        {
            tDFDXIndex = 3 * i;
        }
        else
        {
            throw std::runtime_error("ERROR: Unrecognized formatting for DFDX.");
        }
        tGlobalIDToDFDXMap[tCurGlobalNodeID] = {aDFDX[tDFDXIndex], aDFDX[tDFDXIndex + 1], aDFDX[tDFDXIndex + 2]};
    }
    return tGlobalIDToDFDXMap;
}

}  // namespace Plato::Krino
