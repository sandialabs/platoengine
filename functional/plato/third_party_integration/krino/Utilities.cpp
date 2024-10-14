#include "plato/third_party_integration/krino/Utilities.hpp"

#include <Akri_DiagWriter.hpp>
#include <stk_util/diag/WriterRegistry.hpp>
#include <stk_util/environment/Env.hpp>
#include <stk_util/environment/EnvData.hpp>
#include <stk_util/environment/OutputLog.hpp>

#include "plato/utilities/Enumerate.hpp"
#include "plato/utilities/Exception.hpp"

namespace plato::third_party_integration::krino
{

namespace
{
constexpr int kNumDimensions = 3;
constexpr std::string_view kKrinoLogName = "krinolog";
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
            tDFDXIndex = kNumDimensions * (tCurGlobalNodeID - 1);
        }
        else if (aDFDXFormatting == DFDXFormatting::OneToN)
        {
            tDFDXIndex = kNumDimensions * tIndex;
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
            for (size_t w = 0; w < kNumDimensions; ++w)
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
