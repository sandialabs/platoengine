#include "plato/geometry/extension/KrinoWrapper.hpp"

#include <Akri_MeshInterface.hpp>
#include <Akri_Phase_Support.hpp>
#include <algorithm>
#include <boost/mpi/collectives.hpp>
#include <cstddef>
#include <filesystem>
#include <iterator>
#include <memory>
#include <optional>
#include <set>
#include <stk_mesh/base/Entity.hpp>
#include <stk_mesh/base/Types.hpp>
#include <stk_topology/topology_decl.hpp>
#include <unordered_map>
#include <utility>
#include <vector>

#include "plato/analysis/AnalysisDomainMesh.hpp"
#include "plato/analysis/AnalysisDomainMeshSequentialView.hpp"
#include "plato/components/ComponentType.hpp"
#include "plato/mesh/Mesh.hpp"
#include "plato/services/ScopedExternalRedirectLogger.hpp"
#include "plato/third_party_integration/common/Vector3.hpp"
#include "plato/third_party_integration/krino/BackgroundMesh.hpp"
#include "plato/third_party_integration/krino/LevelSetPrimitives.hpp"
#include "plato/third_party_integration/krino/SensitivityMapUtilities.hpp"
#include "plato/third_party_integration/krino/SnappingParameters.hpp"
#include "plato/third_party_integration/krino/Utilities.hpp"
#include "plato/third_party_integration/stk_io/ReadUtilities.hpp"  //spatial_dimensions
#include "plato/utilities/ContainerHelpers.hpp"
#include "plato/utilities/Enumerate.hpp"
#include "plato/utilities/MultiVectorView.hpp"
#include "plato/utilities/NamedType.hpp"
#include "plato/utilities/ReduceUtilities.hpp"
#include "plato/utilities/TransformIf.hpp"
#include "plato/utilities/Zip.hpp"

namespace plato::geometry::extension
{
namespace
{
namespace tpik = third_party_integration::krino;

void set_level_set_fields(const tpik::BackgroundMesh& aBackgroundMesh,
                          const analysis::AnalysisDomainMesh& aAnalysisDomainMesh)
{
    for (const analysis::ScalarFieldValue tScalarFieldValue :
         analysis::AnalysisDomainMeshSequentialView{aAnalysisDomainMesh})
    {
        const auto tStkEntity = aBackgroundMesh.mKrinoMesh->bulk_data().get_entity(
            stk::topology::NODE_RANK, tScalarFieldValue.mGlobalMeshEntityID);

        if (tStkEntity != stk::mesh::Entity::InvalidEntity)
        {
            tpik::level_set_value(aBackgroundMesh.mLevelSetFields, tStkEntity) = tScalarFieldValue.mValue;
        }
    }
}

[[nodiscard]] auto cut_mesh_compute_sensitivities(
    const tpik::BackgroundMesh& aBackgroundMesh,
    const std::vector<tpik::BackgroundMeshNodeId>& aDesignDomainBackgroundNodes,
    const tpik::SnappingParameters aSnappingParameters) -> tpik::SensitivityMap
{
    [[maybe_unused]] const auto tScopedLogger =
        services::ScopedExternalRedirectLogger{components::ComponentType::kGeometry, "krino-wrapper"};

    tpik::cut_mesh(aBackgroundMesh, aSnappingParameters);
    return detail::compute_sensitivities(aBackgroundMesh, aDesignDomainBackgroundNodes);
}

}  // namespace

KrinoWrapper::KrinoWrapper(third_party_integration::krino::BackgroundMesh aBackgroundMesh,
                           const third_party_integration::krino::VoidPhase aVoidPhase,
                           const tpik::SnappingParameters aSnappingParameters)
    : mBackgroundMesh(std::move(aBackgroundMesh)),
      mNumberOfDesignDomainBackgroundNodes(tpik::background_node_ids(mBackgroundMesh).size()),
      mSensitivityMap(cut_mesh_compute_sensitivities(
          mBackgroundMesh, tpik::background_node_ids(mBackgroundMesh), aSnappingParameters)),
      mSelector(tpik::create_output_selector(mBackgroundMesh.mKrinoMesh->bulk_data(), aVoidPhase)),
      mVoidPhase(aVoidPhase)

{
}

void KrinoWrapper::writeCutMesh(const std::filesystem::path& aFileName) const
{
    tpik::write_mesh(mBackgroundMesh.mKrinoMesh->bulk_data(), aFileName, mSelector);
}

auto KrinoWrapper::sensitivities() const -> const tpik::SensitivityMap& { return mSensitivityMap; }

namespace
{
using ParentIndex = utilities::NamedType<long unsigned int, struct ParentIndexTag>;
using CutMeshIndex = utilities::NamedType<long unsigned int, struct CutMeshIndexTag>;

const auto kJacobianImpl = [](utilities::MultiVectorView<std::vector<double>>& aResultVectorView,
                              const std::vector<double>& aRowVector,
                              const ParentIndex aParentIndex,
                              const CutMeshIndex aCutMeshIndex,
                              const double aMultiplicityMultiplier,
                              const third_party_integration::common::Vector3& aSensitivity,
                              const unsigned int aSpatialDimension)
{
    constexpr auto tScalarViewComponent = utilities::ComponentIndex{0};
    const auto tRowvector3 =
        detail::row_vector_to_vector3(aRowVector, utilities::VectorIndex{aCutMeshIndex.mValue}, aSpatialDimension);
    const auto tDotProduct = third_party_integration::common::dot(tRowvector3, aSensitivity) * aMultiplicityMultiplier;

    aResultVectorView(utilities::VectorIndex{aParentIndex.mValue}, tScalarViewComponent) += tDotProduct;
};

const auto kAdjointJacobianImpl = [](utilities::MultiVectorView<std::vector<double>>& aResultVectorView,
                                     const std::vector<double>& aRowVector,
                                     const ParentIndex aParentIndex,
                                     const CutMeshIndex aCutMeshIndex,
                                     const double aMultiplicityMultiplier,
                                     const third_party_integration::common::Vector3& aSensitivity,
                                     const unsigned int /*aSpatialDimension*/)
{
    aResultVectorView(utilities::VectorIndex{aCutMeshIndex.mValue}) +=
        aSensitivity * aRowVector[aParentIndex.mValue] * aMultiplicityMultiplier;
};

using ResultSize = utilities::NamedType<long unsigned int, struct ResultSizeTag>;
using ResultViewDimensionality = utilities::NamedType<unsigned int, struct ResultViewDimensionalityTag>;

/// @brief Computes a row vector product based on the ApplyFunction @a aApplyFunction which will either be the
/// Jacobian or the Adjoint Jacobian
template <typename Lambda>
[[nodiscard]] auto transform_sensitivity_map(const std::vector<double>& aRowVector,
                                             const tpik::SensitivityMap& aSensitivityMap,
                                             const ::krino::MeshInterface& aKrinoMesh,
                                             const tpik::VoidPhase aVoidPhase,
                                             const ResultSize aResultSize,
                                             const ResultViewDimensionality aResultViewDimensionality,
                                             const Lambda& aApplyFunction) -> std::vector<double>
{
    const auto tSpatialDimensions = third_party_integration::stk_io::spatial_dimensions(aKrinoMesh.bulk_data());
    const auto tCutMeshNodeIds = tpik::cut_mesh_node_ids(aKrinoMesh, aVoidPhase);
    const auto tCutMeshMultiplicity = tpik::cut_mesh_node_id_multiplicity(aSensitivityMap);

    auto tRowVectorMatrixProduct = std::vector<double>(aResultSize.mValue, 0.0);
    auto tRowVectorMatrixProductView =
        utilities::make_multi_vector_view(tRowVectorMatrixProduct, aResultViewDimensionality.mValue);

    for (const auto [tIndex, tCutMeshId] : utilities::enumerate(tCutMeshNodeIds))
    {
        if (const auto tSensitivityMapAtCutMeshIdIterator = aSensitivityMap.find(tCutMeshId);
            tSensitivityMapAtCutMeshIdIterator != aSensitivityMap.end())
        {
            const double tMultiplicityMultiplier = tCutMeshMultiplicity.find(tCutMeshId) != tCutMeshMultiplicity.end()
                                                       ? 1.0 / tCutMeshMultiplicity.at(tCutMeshId)
                                                       : 1.0;

            const auto& tLevelSetJacobianColumn = tSensitivityMapAtCutMeshIdIterator->second;

            for (const auto& [tParentId, tSensitivity, tLocalParentIndex] : utilities::Zip(
                     tLevelSetJacobianColumn.mBackgroundMeshNodeIDs, tLevelSetJacobianColumn.mNodalSensitivities,
                     tLevelSetJacobianColumn.mDesignDomainLocalIndex))
            {
                aApplyFunction(tRowVectorMatrixProductView, aRowVector, ParentIndex{tLocalParentIndex},
                               CutMeshIndex{tIndex}, tMultiplicityMultiplier, tSensitivity, tSpatialDimensions);
            }
        }
    }

    return utilities::reduce_vector(tRowVectorMatrixProduct, tpik::retrieve_mpi_communicator_from_krino());
}

}  // namespace

auto KrinoWrapper::rowVectorJacobianProduct(const std::vector<double>& aCutMeshRowVector) const -> std::vector<double>
{
    const auto tViewDimension = 1U;
    const auto tResultSize = mNumberOfDesignDomainBackgroundNodes;
    return transform_sensitivity_map(aCutMeshRowVector, mSensitivityMap, *mBackgroundMesh.mKrinoMesh, mVoidPhase,
                                     ResultSize{tResultSize}, ResultViewDimensionality{tViewDimension}, kJacobianImpl);
}

auto KrinoWrapper::rowVectorAdjointJacobianProduct(const std::vector<double>& aBackgroundMeshRowVector) const
    -> std::vector<double>
{
    const auto tViewDimension =
        third_party_integration::stk_io::spatial_dimensions(mBackgroundMesh.mKrinoMesh->bulk_data());
    const auto tResultSize = tpik::cut_mesh_node_ids(*mBackgroundMesh.mKrinoMesh, mVoidPhase).size() * tViewDimension;
    return transform_sensitivity_map(aBackgroundMeshRowVector, mSensitivityMap, *mBackgroundMesh.mKrinoMesh, mVoidPhase,
                                     ResultSize{tResultSize}, ResultViewDimensionality{tViewDimension},
                                     kAdjointJacobianImpl);
}

namespace
{

[[nodiscard]] auto down_select_to_design_domain(
    const std::unordered_map<tpik::BackgroundMeshNodeId, double>& aLevelSetValuesMap,
    const std::vector<tpik::BackgroundMeshNodeId>& aBackgroundDesignIDs) -> std::vector<double>
{
    const auto tFoundCondition = [&aLevelSetValuesMap](const auto aDesignDomainId) -> bool
    { return aLevelSetValuesMap.find(aDesignDomainId) != aLevelSetValuesMap.end(); };

    auto tLevelSetValues = utilities::reserved_container<std::vector<double>>(aBackgroundDesignIDs.size());
    utilities::transform_if(
        aBackgroundDesignIDs, std::back_inserter(tLevelSetValues), [&aLevelSetValuesMap](const auto aBackgroundId)
        { return aLevelSetValuesMap.at(aBackgroundId); }, tFoundCondition);

    return tLevelSetValues;
}

}  // namespace

auto make_initial_guess_from_level_set_primitives(const std::filesystem::path& aFileName,
                                                  const tpik::LevelSetPrimitives& aLevelSetPrimitives,
                                                  const std::set<std::string>& aFixedBlocks,
                                                  const LevelSetBounds& aBounds) -> std::vector<double>
{
    const auto tBackgroundMesh = tpik::populate_level_set_field_from_primitives(
        aLevelSetPrimitives, tpik::initialize_background_mesh(aFileName, aFixedBlocks));
    const auto tLevelSetValuesMap = tpik::get_level_set_values(tBackgroundMesh);
    const auto tBackgroundNodeIds = tpik::background_node_ids(tBackgroundMesh);

    return detail::clamp_to_bounds(down_select_to_design_domain(tLevelSetValuesMap, tBackgroundNodeIds), aBounds);
}

auto make_krino_wrapper_from_analysis_domain_mesh(const analysis::AnalysisDomainMesh& aAnalysisDomainMesh,
                                                  const std::set<std::string>& aFixedBlocks,
                                                  const tpik::VoidPhase& aVoidPhase,
                                                  const tpik::SnappingParameters aSnappingParameters) -> KrinoWrapper
{
    auto tBackgroundMesh = tpik::initialize_background_mesh(aAnalysisDomainMesh.mFileName, aFixedBlocks);
    set_level_set_fields(tBackgroundMesh, aAnalysisDomainMesh);
    return KrinoWrapper{std::move(tBackgroundMesh), aVoidPhase, aSnappingParameters};
}

namespace detail
{
namespace
{

void add_if_found(tpik::LevelSetJacobianColumn& aLevelSetJacobianColumn,
                  const tpik::BackgroundMeshNodeId aBackgroundMeshNodeId,
                  const third_party_integration::common::Vector3& aVector3,
                  const std::vector<tpik::BackgroundMeshNodeId>& aDesignDomainBackgroundNodes)
{
    const auto tParentIterator = std::lower_bound(aDesignDomainBackgroundNodes.begin(),
                                                  aDesignDomainBackgroundNodes.end(), aBackgroundMeshNodeId);

    if (tParentIterator != aDesignDomainBackgroundNodes.end() && *tParentIterator == aBackgroundMeshNodeId)
    {
        aLevelSetJacobianColumn.mBackgroundMeshNodeIDs.push_back(aBackgroundMeshNodeId);
        aLevelSetJacobianColumn.mNodalSensitivities.push_back(aVector3);

        aLevelSetJacobianColumn.mDesignDomainLocalIndex.push_back(
            std::distance(aDesignDomainBackgroundNodes.begin(), tParentIterator));
    }
}

[[nodiscard]] auto make_level_set_jacobian_column(
    const std::vector<tpik::BackgroundMeshNodeId>& aParentNodeIds,
    const std::vector<third_party_integration::common::Vector3>& aSensitivities,
    const std::vector<tpik::BackgroundMeshNodeId>& aDesignDomainBackgroundNodes)
    -> std::optional<tpik::LevelSetJacobianColumn>
{
    tpik::LevelSetJacobianColumn tLevelSetJacobianColumn;
    for (const auto& [tParentId, tSensitivity] : utilities::Zip(aParentNodeIds, aSensitivities))
    {
        add_if_found(tLevelSetJacobianColumn, tParentId, tSensitivity, aDesignDomainBackgroundNodes);
    }

    if (!tLevelSetJacobianColumn.mBackgroundMeshNodeIDs.empty())
    {
        return tLevelSetJacobianColumn;
    }
    return std::nullopt;
}

}  // namespace

auto compute_sensitivities(const tpik::BackgroundMesh& aBackgroundMesh,
                           const std::vector<tpik::BackgroundMeshNodeId>& aDesignDomainBackgroundNodes)
    -> tpik::SensitivityMap
{
    const auto tSensitivitiesFromKrino = tpik::get_krino_sensitivities(aBackgroundMesh);
    const auto tSpatialDimension = aBackgroundMesh.mKrinoMesh->bulk_data().mesh_meta_data().spatial_dimension();
    auto tSensitivityMap = utilities::reserved_container<tpik::SensitivityMap>(tSensitivitiesFromKrino.size());
    for (const auto& tCurrentSensitivity : tSensitivitiesFromKrino)
    {
        const auto tSensitivity = tpik::coordinate_level_set_sensitivity(tCurrentSensitivity, tSpatialDimension);
        if (auto tLevelSetJacobianColumn = make_level_set_jacobian_column(tCurrentSensitivity.parentNodeIds,
                                                                          tSensitivity, aDesignDomainBackgroundNodes))
        {
            tSensitivityMap[tCurrentSensitivity.interfaceNodeId] = std::move(tLevelSetJacobianColumn).value();
        }
    }
    return tSensitivityMap;
}

auto row_vector_to_vector3(const std::vector<double>& aRowVector,
                           const utilities::VectorIndex aVectorIndex,
                           const std::size_t aDimensions) -> third_party_integration::common::Vector3
{
    const auto tRowVectorView = utilities::make_multi_vector_view(aRowVector, aDimensions);
    return tRowVectorView(aVectorIndex);
}

auto clamp_to_bounds(const std::vector<double>& aValues, const LevelSetBounds& aBounds) -> std::vector<double>
{
    auto tClamped = utilities::reserved_container<std::vector<double>>(aValues.size());
    std::ranges::transform(aValues, std::back_inserter(tClamped), [&aBounds](const auto aValue)
                           { return std::max(aBounds.mValue.first, std::min(aBounds.mValue.second, aValue)); });
    return tClamped;
}

}  // namespace detail

}  // namespace plato::geometry::extension
