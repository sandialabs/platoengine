#include "plato/third_party_integration/krino/Interface.hpp"

#include <numeric>

#include "plato/analysis/AnalysisDomainMesh.hpp"
#include "plato/analysis/AnalysisDomainMeshRandomAccessView.hpp"
#include "plato/third_party_integration/krino/KrinoWrapper.hpp"
#include "plato/third_party_integration/krino/LevelSetPrimitives.hpp"
#include "plato/utilities/IndexRange.hpp"
#include "plato/utilities/MultiVectorView.hpp"
#include "plato/utilities/Zip.hpp"

namespace plato::third_party_integration::krino
{
namespace
{
constexpr auto kNumDimensions = std::size_t{3};

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
                          const LevelSetJacobianColumn &aLevelSetJacobianColumn,
                          const utilities::VectorIndex aVectorIndex) -> analysis::AnalysisDomainMesh
{
    const auto tDFDLSRandomAccessView = analysis::AnalysisDomainMeshMutableRandomAccessView{aDFDLS};
    for (const auto &[tParentNodeBackgroundID, tParentNodeDXDP] :
         utilities::Zip{aLevelSetJacobianColumn.mBackgroundMeshNodeIDs, aLevelSetJacobianColumn.mNodalSensitivities})
    {
        auto tDFDLSRandomAccessViewValue = tDFDLSRandomAccessView[tParentNodeBackgroundID];
        tDFDLSRandomAccessViewValue = static_cast<analysis::ScalarFieldValue>(tDFDLSRandomAccessViewValue).mValue +
                                      dfdls_entry_contribution(aDFDX, aVectorIndex, tParentNodeDXDP);
    }
    return aDFDLS;
}
}  // namespace

auto generate_computational_mesh(const BackgroundMeshFilePath &aBackgroundMeshName,
                                 const CutMeshFilePath &aCutMesh,
                                 const std::vector<double> &aLevelSetValues,
                                 const VoidPhase aVoidRegion)
    -> std::unordered_map<stk::mesh::EntityId, LevelSetJacobianColumn>
{
    KrinoWrapper tKrinoWrapper(aBackgroundMeshName.mValue, aLevelSetValues, aVoidRegion);
    tKrinoWrapper.writeMesh(aCutMesh.mValue);
    return tKrinoWrapper.sensitivities();
}

std::vector<double> initialize_mesh_with_level_set_primitives(const BackgroundMeshFilePath &aBackgroundMeshName,
                                                              const CutMeshFilePath &aCutMesh,
                                                              const LevelSetPrimitives &aLevelSetPrimitives,
                                                              const VoidPhase aVoidRegion)
{
    KrinoWrapper tKrinoWrapper(aBackgroundMeshName.mValue, aLevelSetPrimitives, aVoidRegion);
    tKrinoWrapper.writeMesh(aCutMesh.mValue);
    return tKrinoWrapper.levelsetValues();
}

auto calculate_dfdls(const std::vector<double> &aDFDX,
                     const analysis::AnalysisDomainMesh &aCutMeshSpaceIDs,
                     const std::unordered_map<stk::mesh::EntityId, LevelSetJacobianColumn> &aDXDP,
                     analysis::AnalysisDomainMesh &&aBackgroundMeshSpaceIDs) -> analysis::AnalysisDomainMesh
{
    const auto tCutMeshSpaceRandomAccessView = analysis::AnalysisDomainMeshRandomAccessView{aCutMeshSpaceIDs};
    for (const auto &[tCurInterfaceNodeID, tLevelSetJacobianColumn] : aDXDP)
    {
        const auto tCutMeshScalarFieldValues = tCutMeshSpaceRandomAccessView[tCurInterfaceNodeID];
        assert(tCutMeshScalarFieldValues.has_value());
        const auto tVectorIndex = utilities::VectorIndex{tCutMeshScalarFieldValues.value().mDesignVariableVectorIndex};
        aBackgroundMeshSpaceIDs =
            assemble_dfdls_entry(std::move(aBackgroundMeshSpaceIDs), aDFDX, tLevelSetJacobianColumn, tVectorIndex);
    }
    return aBackgroundMeshSpaceIDs;
}

auto calculate_adjoint_dfdls(const analysis::AnalysisDomainMesh &aBackgroundLevelSetSpaceVector,
                             const std::unordered_map<stk::mesh::EntityId, LevelSetJacobianColumn> &aDXDP)
    -> std::unordered_map<KrinoGlobalNodeID, stk::math::Vector3d>
{
    const auto tLevelSetSpaceRandomAccessView =
        analysis::AnalysisDomainMeshRandomAccessView{aBackgroundLevelSetSpaceVector};

    auto tAdjointResult = std::unordered_map<KrinoGlobalNodeID, stk::math::Vector3d>{};
    tAdjointResult.reserve(aDXDP.size());
    for (const auto &[tCurInterfaceNodeID, tLevelSetJacobianColumn] : aDXDP)
    {
        const auto tParentNodeInfo =
            utilities::Zip{tLevelSetJacobianColumn.mBackgroundMeshNodeIDs, tLevelSetJacobianColumn.mNodalSensitivities};
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
