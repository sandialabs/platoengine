#include "plato/third_party_integration/krino/Utilities.hpp"

#include <Akri_BoundingBoxMesh.hpp>
#include <Akri_CDFEM_Support.hpp>
#include <Akri_DiagWriter.hpp>
#include <Akri_LevelSet.hpp>
#include <Akri_MeshHelpers.hpp>
#include <Akri_OutputUtils.hpp>
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
constexpr auto kNumDimensions = std::size_t{3};
constexpr std::string_view kKrinoLogName = "krinolog";
const std::string kLevelsetName = "LS";

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

auto assemble_dfdls_entry(analysis::AnalysisDomainMesh &&aDFDLS,
                          const std::vector<double> &aDFDX,
                          const InterfaceNodeDXDP &aInterfaceNodeDXDP,
                          const utilities::VectorIndex aVectorIndex) -> analysis::AnalysisDomainMesh
{
    const auto tDFDLSRandomAccessView = analysis::AnalysisDomainMeshMutableRandomAccessView{aDFDLS};
    for (const auto &[tParentNodeBackgroundID, tParentNodeDXDP] :
         utilities::Zip{aInterfaceNodeDXDP.mParentNodeIds, aInterfaceNodeDXDP.mParentDXDP})
    {
        auto tDFDLSRandomAccessViewValue = tDFDLSRandomAccessView[tParentNodeBackgroundID];
        tDFDLSRandomAccessViewValue = static_cast<analysis::ScalarFieldValue>(tDFDLSRandomAccessViewValue).mValue +
                                      dfdls_entry_contribution(aDFDX, aVectorIndex, tParentNodeDXDP);
    }
    return aDFDLS;
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
                     analysis::AnalysisDomainMesh &&aBackgroundMeshSpaceIDs) -> analysis::AnalysisDomainMesh
{
    const auto tCutMeshSpaceRandomAccessView = analysis::AnalysisDomainMeshRandomAccessView{aCutMeshSpaceIDs};
    for (const auto &[tCurInterfaceNodeID, tInterfaceNodeDXDP] : aDXDP)
    {
        const auto tCutMeshScalarFieldValues = tCutMeshSpaceRandomAccessView[tCurInterfaceNodeID];
        assert(tCutMeshScalarFieldValues.has_value());
        const auto tVectorIndex = utilities::VectorIndex{tCutMeshScalarFieldValues.value().mDesignVariableVectorIndex};
        aBackgroundMeshSpaceIDs =
            assemble_dfdls_entry(std::move(aBackgroundMeshSpaceIDs), aDFDX, tInterfaceNodeDXDP, tVectorIndex);
    }
    return aBackgroundMeshSpaceIDs;
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

void setup_fields_for_conforming_decomposition(const stk::mesh::MetaData &aMeta)
{
    ::krino::CDFEM_Support &tCdfemSupport = ::krino::CDFEM_Support::get(aMeta);
    const ::krino::FieldRef tCoordsField = aMeta.coordinate_field();

    tCdfemSupport.set_coords_field(tCoordsField);
    tCdfemSupport.add_edge_interpolation_field(tCoordsField);
    tCdfemSupport.register_parent_node_ids_field();
}

void create_bounding_box_mesh(const stk::math::Vector3d &aMinCorner,
                              const stk::math::Vector3d &aMaxCorner,
                              const double aMeshSize,
                              const std::filesystem::path &aFilename)
{
    auto tBoundingBoxMesh =
        std::make_unique<::krino::BoundingBoxMesh>(stk::topology::TET_4, stk::EnvData::parallel_comm());
    ::krino::LevelSet &tLevelSet =
        ::krino::LevelSet::build(tBoundingBoxMesh->meta_data(), kLevelsetName, sierra::Diag::sierraTimer());
    tLevelSet.set_distance_name(kLevelsetName);
    tLevelSet.setup();
    setup_fields_for_conforming_decomposition(tBoundingBoxMesh->meta_data());
    tBoundingBoxMesh->set_domain(::krino::BoundingBoxMesh::BoundingBoxType(aMinCorner, aMaxCorner), aMeshSize);
    tBoundingBoxMesh->set_mesh_structure_type(::krino::FLAT_WALLED_BCC_BOUNDING_BOX_MESH);
    tBoundingBoxMesh->populate_mesh();
    ::krino::activate_all_entities(tBoundingBoxMesh->bulk_data(),
                                   ::krino::AuxMetaData::get(tBoundingBoxMesh->meta_data()).active_part());
    ::krino::output_composed_mesh_with_fields(tBoundingBoxMesh->bulk_data(),
                                              ::krino::AuxMetaData::get(tBoundingBoxMesh->meta_data()).active_part(),
                                              aFilename.string(), 1, 0.0);
}

}  // namespace plato::third_party_integration::krino
