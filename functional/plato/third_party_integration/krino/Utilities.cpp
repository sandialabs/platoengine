#include "Utilities.hpp"

#include <Akri_DiagWriter.hpp>
#include <stk_util/diag/WriterRegistry.hpp>
#include <stk_util/environment/Env.hpp>
#include <stk_util/environment/EnvData.hpp>
#include <stk_util/environment/OutputLog.hpp>

#include "plato/utilities/Enumerate.hpp"

namespace plato::third_party_integration::krino
{

namespace
{
constexpr std::string_view kKrinoLogName = "krinolog";
constexpr int kNumDimensions = 3;
}  // namespace

void initialize_environment_for_krino(const MPI_Comm &aComm)
{
    // Initialize STK environment
    stk::EnvData::instance().m_parallelComm = aComm;
    MPI_Comm_size(stk::EnvData::parallel_comm(), &stk::EnvData::instance().m_parallelSize);
    MPI_Comm_rank(stk::EnvData::parallel_comm(), &stk::EnvData::instance().m_parallelRank);

    // Initialize krion logging
    sierra::Diag::registerWriter(static_cast<std::string>(kKrinoLogName), ::krinolog, ::krino::theDiagWriterParser());
    const std::string output_description = "out>pout dout>out";
    const std::string parallel_output_description = " pout>null";
    stk::bind_output_streams(output_description + parallel_output_description);
}

std::pair<double, double> calculate_overlapping_single_sphere_locator_data(const SpherePatternData &aPatternData,
                                                                           const size_t &aDimension)
{
    std::pair<double, double> tStartAndSpacing(
        aPatternData.mBoundingBoxMinXYZ[aDimension],
        (aPatternData.mBoundingBoxMaxXYZ[aDimension] - aPatternData.mBoundingBoxMinXYZ[aDimension]) / 2.0);
    return tStartAndSpacing;
}

std::pair<double, double> calculate_overlapping_many_sphere_locator_data(const SpherePatternData &aPatternData,
                                                                         const size_t &aDimension)
{
    const double tSpacing =
        (aPatternData.mBoundingBoxMaxXYZ[aDimension] - aPatternData.mBoundingBoxMinXYZ[aDimension]) /
        (aPatternData.mNumSpheres[aDimension] - 1);
    std::pair<double, double> tStartAndSpacing(aPatternData.mBoundingBoxMinXYZ[aDimension] - tSpacing, tSpacing);
    return tStartAndSpacing;
}

std::pair<double, double> calculate_non_overlapping_sphere_locator_data(const SpherePatternData &aPatternData,
                                                                        const size_t &aDimension)
{
    std::pair<double, double> tStartAndSpacing(
        aPatternData.mBoundingBoxMinXYZ[aDimension],
        (aPatternData.mBoundingBoxMaxXYZ[aDimension] - aPatternData.mBoundingBoxMinXYZ[aDimension]) /
            (aPatternData.mNumSpheres[aDimension] + 1));
    return tStartAndSpacing;
}

SphereLocatorData calculate_sphere_starts_and_spacing(const SpherePatternData &aData)
{
    SphereLocatorData tLocatorData(kNumDimensions, 0.0);
    for (size_t tCurDimension = 0; tCurDimension < kNumDimensions; ++tCurDimension)
    {
        if (aData.mSpheresCanOverlapBoundingBox)
        {
            if (aData.mNumSpheres[tCurDimension] == 1)
            {
                tLocatorData.mStartAndSpacing[tCurDimension] =
                    calculate_overlapping_single_sphere_locator_data(aData, tCurDimension);
            }
            else
            {
                tLocatorData.mStartAndSpacing[tCurDimension] =
                    calculate_overlapping_many_sphere_locator_data(aData, tCurDimension);
            }
        }
        else
        {
            tLocatorData.mStartAndSpacing[tCurDimension] =
                calculate_non_overlapping_sphere_locator_data(aData, tCurDimension);
        }
    }
    return tLocatorData;
}

std::vector<double> calculate_sphere_center_coords(int aNumValues, double aStart, double aStep)
{
    std::vector<double> tValues(aNumValues);
    std::generate(tValues.begin(), tValues.end(), [&aStart, &aStep] { return aStart += aStep; });
    return tValues;
}

std::vector<Sphere> generate_spheres(const SpherePatternData &aData)
{
    SphereLocatorData tLocatorData = calculate_sphere_starts_and_spacing(aData);

    std::vector<double> tXValues = calculate_sphere_center_coords(aData.mNumSpheres[Dimension::X],
                                                                  tLocatorData.mStartAndSpacing[Dimension::X].first,
                                                                  tLocatorData.mStartAndSpacing[Dimension::X].second);
    std::vector<double> tYValues = calculate_sphere_center_coords(aData.mNumSpheres[Dimension::Y],
                                                                  tLocatorData.mStartAndSpacing[Dimension::Y].first,
                                                                  tLocatorData.mStartAndSpacing[Dimension::Y].second);
    std::vector<double> tZValues = calculate_sphere_center_coords(aData.mNumSpheres[Dimension::Z],
                                                                  tLocatorData.mStartAndSpacing[Dimension::Z].first,
                                                                  tLocatorData.mStartAndSpacing[Dimension::Z].second);

    // Loop to create 3D array of spheres
    std::vector<Sphere> tSpheres;
    tSpheres.reserve(aData.mNumSpheres[Dimension::X] * aData.mNumSpheres[Dimension::Y] *
                     aData.mNumSpheres[Dimension::Z]);

    for (auto tCurX : tXValues)
    {
        for (auto tCurY : tYValues)
        {
            for (auto tCurZ : tZValues)
            {
                tSpheres.push_back(Sphere{tCurX, tCurY, tCurZ, aData.mSphereRadius});
            }
        }
    }
    return tSpheres;
}

std::unordered_map<KrinoGlobalNodeID, stk::math::Vector3d> assemble_global_id_to_dfdx_map(
    const std::vector<double> &aDFDX,
    const std::vector<KrinoGlobalNodeID> &aCutMeshGlobalNodeIDMap,
    const DFDXFormatting aDFDXFormatting)
{
    std::unordered_map<KrinoGlobalNodeID, stk::math::Vector3d> tGlobalIDToDFDXMap;
    for (const auto &[tIndex, tCurGlobalNodeID] : utilities::enumerate(aCutMeshGlobalNodeIDMap))
    {
        KrinoGlobalNodeID tDFDXIndex = 0;
        if (aDFDXFormatting == DFDXFormatting::GlobalID)
        {
            tDFDXIndex = 3 * (tCurGlobalNodeID - 1);
        }
        else if (aDFDXFormatting == DFDXFormatting::OneToN)
        {
            tDFDXIndex = 3 * tIndex;
        }
        else
        {
            throw std::runtime_error("ERROR: Unrecognized formatting for DFDX.");
        }
        tGlobalIDToDFDXMap[tCurGlobalNodeID] = {aDFDX[tDFDXIndex], aDFDX[tDFDXIndex + 1], aDFDX[tDFDXIndex + 2]};
    }
    return tGlobalIDToDFDXMap;
}

std::unordered_map<KrinoGlobalNodeID, double> calculate_dfdls(
    const std::unordered_map<KrinoGlobalNodeID, stk::math::Vector3d> &aDFDXMap,
    const std::unordered_map<stk::mesh::EntityId, InterfaceNodeDXDP> &aDXDP,
    const std::vector<KrinoGlobalNodeID> &aBackgroundNodemap)
{
    std::unordered_map<KrinoGlobalNodeID, double> tDFDLS;
    for (auto tNodeID : aBackgroundNodemap)
    {
        tDFDLS[tNodeID] = 0.0;
    }
    std::unordered_map<stk::mesh::EntityId, InterfaceNodeDXDP>::const_iterator tDXDPMapIter = aDXDP.begin();
    while (tDXDPMapIter != aDXDP.end())
    {
        const KrinoGlobalNodeID tCurInterfaceNodeID = tDXDPMapIter->first;
        if (aDFDXMap.count(tCurInterfaceNodeID) == 0)
        {
            throw utilities::Exception(
                "ERROR: Cut mesh interface global node id does not have a corresponding DFDX entry!");
        }

        for (size_t j = 0; j < tDXDPMapIter->second.mParentNodeIds.size(); ++j)
        {
            const KrinoGlobalNodeID tCurBackgroundMeshNodeID = tDXDPMapIter->second.mParentNodeIds[j];
            double tContribution = 0.0;
            for (size_t w = 0; w < 3; ++w)
            {
                tContribution += aDFDXMap.at(tCurInterfaceNodeID)[w] * tDXDPMapIter->second.mParentDXDP[j][w];
            }
            tDFDLS[tCurBackgroundMeshNodeID] += tContribution;
        }
        tDXDPMapIter++;
    }
    return tDFDLS;
}

}  // namespace plato::third_party_integration::krino
