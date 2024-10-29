#include "plato/third_party_integration/krino/Utilities.hpp"

#include <Akri_DiagWriter.hpp>
#include <numeric>
#include <stk_util/diag/WriterRegistry.hpp>
#include <stk_util/environment/Env.hpp>
#include <stk_util/environment/EnvData.hpp>
#include <stk_util/environment/OutputLog.hpp>

#include "plato/analysis/AnalysisDomainMesh.hpp"
#include "plato/analysis/AnalysisDomainMeshRandomAccessView.hpp"
#include "plato/utilities/IndexRange.hpp"
#include "plato/utilities/MultiVectorView.hpp"
#include "plato/utilities/Zip.hpp"

namespace plato::third_party_integration::krino
{

namespace
{
constexpr int kNumDimensions = std::size_t{3};
constexpr std::string_view kKrinoLogName = "krinolog";

auto dfdls_entry_contribution(const std::vector<double> &aDFDX,
                              const utilities::VectorIndex aVectorIndex,
                              const stk::math::Vector3d &aParentNodeDXDP) -> double
{
    const auto tDFDXView = utilities::make_multi_vector_view<kNumDimensions>(aDFDX);
    const auto tComponentRange = utilities::IndexRange{kNumDimensions};
    return std::accumulate(tComponentRange.begin(), tComponentRange.end(), 0.0,
                           [&](const auto tSum, const auto tComponentIndex)
                           {
                               const auto tDFDXComponent =
                                   tDFDXView(aVectorIndex, utilities::ComponentIndex{tComponentIndex});
                               const auto tParentNodeDXDPVectorComponent = aParentNodeDXDP[tComponentIndex];
                               return tSum + tDFDXComponent * tParentNodeDXDPVectorComponent;
                           });
}

void assemble_dfdls_entry(std::unordered_map<KrinoGlobalNodeID, double> &aDFDLS,
                          const std::vector<double> &aDFDX,
                          const InterfaceNodeDXDP &aInterfaceNodeDXDP,
                          const utilities::VectorIndex aVectorIndex)
{
    for (const auto &[tParentNodeBackgroundID, tParentNodeDXDP] :
         utilities::Zip{aInterfaceNodeDXDP.mParentNodeIds, aInterfaceNodeDXDP.mParentDXDP})
    {
        aDFDLS[tParentNodeBackgroundID] += dfdls_entry_contribution(aDFDX, aVectorIndex, tParentNodeDXDP);
    }
}
}  // namespace

void initialize_environment_for_krino(const MPI_Comm &aComm)
{
    // Initialize STK environment
    stk::EnvData::instance().m_parallelComm = aComm;
    MPI_Comm_size(stk::EnvData::parallel_comm(), &stk::EnvData::instance().m_parallelSize);
    MPI_Comm_rank(stk::EnvData::parallel_comm(), &stk::EnvData::instance().m_parallelRank);

    // Initialize krino logging
    sierra::Diag::registerWriter(std::string{kKrinoLogName}, ::krinolog, ::krino::theDiagWriterParser());
    const std::string tOutputDescription = "out>pout dout>out";
    const std::string tParallelOutputDescription = " pout>null";
    stk::bind_output_streams(tOutputDescription + tParallelOutputDescription);
}

auto calculate_dfdls(const std::vector<double> &aDFDX,
                     const analysis::AnalysisDomainMesh &aCutMeshSpaceIDs,
                     const std::unordered_map<stk::mesh::EntityId, InterfaceNodeDXDP> &aDXDP,
                     const std::vector<KrinoGlobalNodeID> &aBackgroundNodemap)
    -> std::unordered_map<KrinoGlobalNodeID, double>
{
    const auto tCutMeshSpaceRandomAccessView = analysis::AnalysisDomainMeshRandomAccessView{aCutMeshSpaceIDs};
    auto tDFDLS = std::unordered_map<KrinoGlobalNodeID, double>{};
    tDFDLS.reserve(aBackgroundNodemap.size());
    for (const auto tNodeID : aBackgroundNodemap)
    {
        tDFDLS[tNodeID] = 0.0;
    }
    for (const auto &[tCurInterfaceNodeID, tInterfaceNodeDXDP] : aDXDP)
    {
        const auto tCutMeshScalarFieldValues = tCutMeshSpaceRandomAccessView[tCurInterfaceNodeID];
        assert(tCutMeshScalarFieldValues.has_value());
        const auto tVectorIndex = utilities::VectorIndex{tCutMeshScalarFieldValues.value().mDesignVariableVectorIndex};
        // Note: This `if` statement is to account for the fact that if Krino is not using the `include_void_region`
        // option, the sensitivities vector may have a size equal to the number of nodes only in the material block, not
        // both material and void blocks. This is a temporary fix and should be handled more robustly.
        const auto tDFDXView = utilities::make_multi_vector_view<kNumDimensions>(aDFDX);
        if (tVectorIndex.mValue < tDFDXView.numberOfVectors())
        {
            assemble_dfdls_entry(tDFDLS, aDFDX, tInterfaceNodeDXDP, tVectorIndex);
        }
    }
    return tDFDLS;
}

auto calculate_adjoint_dfdls(const analysis::AnalysisDomainMesh &aBackgroundLevelSetSpaceVector,
                             const std::unordered_map<stk::mesh::EntityId, InterfaceNodeDXDP> &aDXDP)
    -> std::unordered_map<KrinoGlobalNodeID, stk::math::Vector3d>
{
    const auto tLevelSetSpaceRandomAccessView =
        analysis::AnalysisDomainMeshRandomAccessView{aBackgroundLevelSetSpaceVector};

    auto tAdjointResult = std::unordered_map<KrinoGlobalNodeID, stk::math::Vector3d>{};
    tAdjointResult.reserve(aDXDP.size());
    for (const auto &[tCurInterfaceNodeID, tInterfaceNodeDXDP] : aDXDP)
    {
        const auto tParentNodeInfo = utilities::Zip{tInterfaceNodeDXDP.mParentNodeIds, tInterfaceNodeDXDP.mParentDXDP};
        tAdjointResult[tCurInterfaceNodeID] = std::accumulate(
            tParentNodeInfo.begin(), tParentNodeInfo.end(), stk::math::Vector3d{0.0, 0.0, 0.0},
            [tLevelSetSpaceRandomAccessView](const stk::math::Vector3d &aSum, const auto &aParentNodeInfo)
            {
                constexpr auto tNodeIdIndex = 0;
                constexpr auto tDXDPIndex = 1;
                const auto tBackgroundValue = tLevelSetSpaceRandomAccessView[std::get<tNodeIdIndex>(aParentNodeInfo)];
                assert(tBackgroundValue);
                return aSum + tBackgroundValue.value().mValue * std::get<tDXDPIndex>(aParentNodeInfo);
            });
    }
    return tAdjointResult;
}
}  // namespace plato::third_party_integration::krino
