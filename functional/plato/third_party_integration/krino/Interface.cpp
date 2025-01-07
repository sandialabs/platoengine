#include "plato/third_party_integration/krino/Interface.hpp"

#include <numeric>

#include "plato/analysis/AnalysisDomainMesh.hpp"
#include "plato/analysis/AnalysisDomainMeshRandomAccessView.hpp"
#include "plato/analysis/Utilities.hpp"
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

auto vector_jacobian_product_entry_contribution(const std::vector<double> &aRowVector,
                                                const utilities::VectorIndex aVectorIndex,
                                                const stk::math::Vector3d &aNodalSensitivities) -> double
{
    const auto tRowVectorView = utilities::make_multi_vector_view<kNumDimensions>(aRowVector);
    const auto tComponentRange = utilities::IndexRange{kNumDimensions};
    return std::accumulate(tComponentRange.begin(), tComponentRange.end(), 0.0,
                           [&](const auto tSum, const auto tComponentIndex)
                           {
                               const auto tRowVectorComponent =
                                   tRowVectorView(aVectorIndex, utilities::ComponentIndex{tComponentIndex});
                               const auto tNodalSensitivitiesVectorComponent = aNodalSensitivities[tComponentIndex];
                               return tSum + tRowVectorComponent * tNodalSensitivitiesVectorComponent;
                           });
}

auto assemble_vector_jacobian_product_entry(analysis::AnalysisDomainMesh &&aVectorJacobianProduct,
                                            const std::vector<double> &aRowVector,
                                            const LevelSetJacobianColumn &aLevelSetJacobianColumn,
                                            const utilities::VectorIndex aVectorIndex) -> analysis::AnalysisDomainMesh
{
    const auto tVectorJacobianProductRandomAccessView =
        analysis::AnalysisDomainMeshMutableRandomAccessView{aVectorJacobianProduct};
    for (const auto &[tParentNodeBackgroundID, tNodalSensitivities] :
         utilities::Zip{aLevelSetJacobianColumn.mBackgroundMeshNodeIDs, aLevelSetJacobianColumn.mNodalSensitivities})
    {
        auto tVectorJacobianProductRandomAccessViewValue =
            tVectorJacobianProductRandomAccessView[tParentNodeBackgroundID];
        if (!tVectorJacobianProductRandomAccessViewValue.empty())
        {
            tVectorJacobianProductRandomAccessViewValue =
                static_cast<analysis::ScalarFieldValue>(tVectorJacobianProductRandomAccessViewValue).mValue +
                vector_jacobian_product_entry_contribution(aRowVector, aVectorIndex, tNodalSensitivities);
        }
    }
    return aVectorJacobianProduct;
}
}  // namespace

auto generate_computational_mesh(const analysis::AnalysisDomainMesh &aBackgroundMeshWithLevelSets,
                                 const double aFixedLevelSetValue,
                                 const CutMeshFilePath &aCutMesh,
                                 const VoidPhase aVoidRegion)
    -> std::unordered_map<stk::mesh::EntityId, LevelSetJacobianColumn>
{
    auto tKrinoWrapper = KrinoWrapper{aBackgroundMeshWithLevelSets, aFixedLevelSetValue, aVoidRegion};
    tKrinoWrapper.writeMesh(aCutMesh.mValue);
    return tKrinoWrapper.sensitivities();
}

auto initialize_mesh_with_level_set_primitives(const BackgroundMeshFilePath &aBackgroundMeshName,
                                               const LevelSetPrimitives &aLevelSetPrimitives,
                                               const VoidPhase aVoidRegion) -> std::vector<double>
{
    const auto tKrinoWrapper = KrinoWrapper{aBackgroundMeshName.mValue, aLevelSetPrimitives, aVoidRegion};
    return tKrinoWrapper.levelSetValues();
}

auto level_set_row_vector_jacobian_product(
    const std::vector<double> &aRowVector,
    const analysis::AnalysisDomainMesh &aCutMeshSpaceIDs,
    const std::unordered_map<stk::mesh::EntityId, LevelSetJacobianColumn> &aLevelSetJacobian,
    analysis::AnalysisDomainMesh &&aBackgroundMeshSpaceIDs) -> analysis::AnalysisDomainMesh
{
    aBackgroundMeshSpaceIDs = analysis::zero_scalar_field(std::move(aBackgroundMeshSpaceIDs));

    const auto tCutMeshSpaceRandomAccessView = analysis::AnalysisDomainMeshRandomAccessView{aCutMeshSpaceIDs};
    for (const auto &[tCurInterfaceNodeID, tLevelSetJacobianColumn] : aLevelSetJacobian)
    {
        const auto tCutMeshScalarFieldValues = tCutMeshSpaceRandomAccessView[tCurInterfaceNodeID];
        assert(tCutMeshScalarFieldValues.has_value());
        const auto tVectorIndex = utilities::VectorIndex{tCutMeshScalarFieldValues.value().mDesignVariableVectorIndex};
        aBackgroundMeshSpaceIDs = assemble_vector_jacobian_product_entry(std::move(aBackgroundMeshSpaceIDs), aRowVector,
                                                                         tLevelSetJacobianColumn, tVectorIndex);
    }
    return aBackgroundMeshSpaceIDs;
}

auto level_set_row_vector_adjoint_jacobian_product(const analysis::AnalysisDomainMesh &aBackgroundLevelSetSpaceVector,
                                                   const LevelSetJacobian &aLevelSetJacobian)
    -> std::unordered_map<KrinoGlobalNodeID, stk::math::Vector3d>
{
    const auto tLevelSetSpaceRandomAccessView =
        analysis::AnalysisDomainMeshRandomAccessView{aBackgroundLevelSetSpaceVector};

    auto tAdjointResult = std::unordered_map<KrinoGlobalNodeID, stk::math::Vector3d>{};
    tAdjointResult.reserve(aLevelSetJacobian.size());
    for (const auto &[tCurInterfaceNodeID, tLevelSetJacobianColumn] : aLevelSetJacobian)
    {
        const auto tParentNodeInfo =
            utilities::Zip{tLevelSetJacobianColumn.mBackgroundMeshNodeIDs, tLevelSetJacobianColumn.mNodalSensitivities};
        tAdjointResult[tCurInterfaceNodeID] = std::accumulate(
            tParentNodeInfo.begin(), tParentNodeInfo.end(), stk::math::Vector3d{0.0, 0.0, 0.0},
            [tLevelSetSpaceRandomAccessView](const stk::math::Vector3d &aSum, const auto &aParentNodeInfo)
            {
                constexpr auto tNodeIdIndex = 0;
                const auto tBackgroundValue = tLevelSetSpaceRandomAccessView[std::get<tNodeIdIndex>(aParentNodeInfo)];
                constexpr auto tSensitivityIndex = 1;
                return aSum + tBackgroundValue.value_or(analysis::ScalarFieldValue{}).mValue *
                                  std::get<tSensitivityIndex>(aParentNodeInfo);
            });
    }
    return tAdjointResult;
}

}  // namespace plato::third_party_integration::krino
