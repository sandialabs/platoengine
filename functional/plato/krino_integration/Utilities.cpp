#include "Utilities.hpp"

#include <Akri_DiagWriter.hpp>
#include <stk_util/diag/WriterRegistry.hpp>
#include <stk_util/environment/Env.hpp>
#include <stk_util/environment/EnvData.hpp>
#include <stk_util/environment/OutputLog.hpp>

namespace plato::krino_integration
{

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

std::map<unsigned int, double> calculateDFDLS(const std::map<unsigned int, stk::math::Vector3d> &aDFDXMap,
                                              const std::map<stk::mesh::EntityId, InterfaceNode_DXDP> &aDXDP,
                                              const std::vector<unsigned int> &aBackgroundNodemap)
{
    std::map<unsigned int, double> tDFDLS;
    for (auto tNodeID : aBackgroundNodemap)
    {
        tDFDLS[tNodeID] = 0.0;
    }
    std::map<stk::mesh::EntityId, InterfaceNode_DXDP>::const_iterator tDXDPMapIter = aDXDP.begin();
    while (tDXDPMapIter != aDXDP.end())
    {
        const unsigned int tCurInterfaceNodeID = tDXDPMapIter->first;
        if (aDFDXMap.count(tCurInterfaceNodeID) == 0)
        {
            std::cout << "ERROR: Cut mesh interface global node id does not have a corresponding DFDX entry!"
                      << std::endl;
            throw 1;
        }

        for (size_t j = 0; j < tDXDPMapIter->second.parentNodeIds.size(); ++j)
        {
            const unsigned int tCurBackgroundMeshNodeID = tDXDPMapIter->second.parentNodeIds[j];
            double tContribution = 0.0;
            for (size_t w = 0; w < 3; ++w)
            {
                tContribution += aDFDXMap.at(tCurInterfaceNodeID)[w] * tDXDPMapIter->second.parentDXDP[j][w];
            }
            tDFDLS[tCurBackgroundMeshNodeID] += tContribution;
        }
        tDXDPMapIter++;
    }
    return tDFDLS;
}

}  // namespace plato::krino_integration
