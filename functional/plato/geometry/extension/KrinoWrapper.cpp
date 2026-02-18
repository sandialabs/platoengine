#include "plato/geometry/extension/KrinoWrapper.hpp"

#include <boost/mpi/collectives.hpp>
#include <boost/mpi/communicator.hpp>
#include <boost/serialization/unordered_map.hpp>
#include <boost/serialization/vector.hpp>
#include <cstddef>
#include <stk_mesh/base/Entity.hpp>
#include <stk_mesh/base/Types.hpp>
#include <stk_util/environment/EnvData.hpp>  //get stk mpi env
#include <unordered_map>
#include <vector>

#include "plato/analysis/AnalysisDomainMesh.hpp"
#include "plato/analysis/AnalysisDomainMeshSequentialView.hpp"
#include "plato/mesh/Mesh.hpp"
#include "plato/services/ScopedExternalRedirectLogger.hpp"
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

void set_level_set_fields(::krino::MeshInterface& aKrinoMesh,
                          std::vector<::krino::LS_Field>& aLevelSetFields,
                          const analysis::AnalysisDomainMesh& aAnalysisDomainMesh)
{
    for (const auto& tScalarFieldValueProxy : analysis::AnalysisDomainMeshSequentialView{aAnalysisDomainMesh})
    {
        const auto& tScalarFieldValue = static_cast<analysis::ScalarFieldValue>(tScalarFieldValueProxy);
        const auto tStkEntity =
            aKrinoMesh.bulk_data().get_entity(stk::topology::NODE_RANK, tScalarFieldValue.mGlobalMeshEntityID);

        if (tStkEntity != stk::mesh::Entity::InvalidEntity)
        {
            tpik::level_set_value(aLevelSetFields, tStkEntity) = tScalarFieldValue.mValue;
        }
    }
}

[[nodiscard]] auto cut_mesh_compute_sensitivities(
    stk::mesh::BulkData& aBulkData,
    const std::vector<::krino::LS_Field>& aLevelSetFields,
    const std::vector<tpik::BackgroundMeshNodeId>& aDesignDomainBackgroundNodes,
    const tpik::SnappingParameters aSnappingParameters) -> tpik::SensitivityMap
{
    [[maybe_unused]] const auto tScopedLogger =
        services::ScopedExternalRedirectLogger{components::ComponentType::kGeometry, "krino-wrapper"};

    tpik::cut_mesh(aBulkData, aLevelSetFields, aSnappingParameters);
    return detail::compute_sensitivities(aBulkData, aLevelSetFields, aDesignDomainBackgroundNodes);
}

}  // namespace

KrinoWrapper::KrinoWrapper(std::unique_ptr<::krino::MeshInterface> aKrinoMeshInterface,
                           std::vector<::krino::LS_Field> aLevelSetField,
                           const third_party_integration::krino::VoidPhase aVoidPhase,
                           const tpik::SnappingParameters aSnappingParameters)
    : mKrinoMesh(std::move(aKrinoMeshInterface)),
      mLevelSetFields(std::move(aLevelSetField)),
      mNumberOfDesignDomainBackgroundNodes(tpik::background_node_ids(*mKrinoMesh, mLevelSetFields).size()),
      mSensitivityMap(cut_mesh_compute_sensitivities(mKrinoMesh->bulk_data(),
                                                     mLevelSetFields,
                                                     tpik::background_node_ids(*mKrinoMesh, mLevelSetFields),
                                                     aSnappingParameters)),
      mSelector(tpik::create_output_selector(mKrinoMesh->bulk_data(), aVoidPhase)),
      mVoidPhase(aVoidPhase)

{
}

void KrinoWrapper::writeCutMesh(const std::filesystem::path& aFileName) const
{
    tpik::write_mesh(mKrinoMesh->bulk_data(), aFileName, mSelector);
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
    constexpr auto kScalarViewComponent = utilities::ComponentIndex{0};
    const auto tRowvector3 =
        detail::row_vector_to_vector3(aRowVector, utilities::VectorIndex{aCutMeshIndex.mValue}, aSpatialDimension);
    const auto tDotProduct = third_party_integration::common::dot(tRowvector3, aSensitivity) * aMultiplicityMultiplier;

    aResultVectorView(utilities::VectorIndex{aParentIndex.mValue}, kScalarViewComponent) += tDotProduct;
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
[[nodiscard]] auto transformSensitivityMap(const std::vector<double>& aRowVector,
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
    return transformSensitivityMap(aCutMeshRowVector, mSensitivityMap, *mKrinoMesh, mVoidPhase, ResultSize{tResultSize},
                                   ResultViewDimensionality{tViewDimension}, kJacobianImpl);
}

auto KrinoWrapper::rowVectorAdjointJacobianProduct(const std::vector<double>& aBackgroundMeshRowVector) const
    -> std::vector<double>
{
    const auto tViewDimension = third_party_integration::stk_io::spatial_dimensions(mKrinoMesh->bulk_data());
    const auto tResultSize = tpik::cut_mesh_node_ids(*mKrinoMesh, mVoidPhase).size() * tViewDimension;
    return transformSensitivityMap(aBackgroundMeshRowVector, mSensitivityMap, *mKrinoMesh, mVoidPhase,
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
                                                  const std::set<std::string>& aFixedBlocks) -> std::vector<double>
{
    auto tKrinoMesh = tpik::read_and_setup_for_decomposition(aFileName, aFixedBlocks);
    auto tLevelSetFields = tpik::make_level_set_field_from_primitives(aLevelSetPrimitives, tKrinoMesh->bulk_data());
    const auto tLevelSetValuesMap = tpik::get_level_set_values(*tKrinoMesh, tLevelSetFields);
    const auto tBackgroundNodeIds = tpik::background_node_ids(*tKrinoMesh, tLevelSetFields);

    return down_select_to_design_domain(tLevelSetValuesMap, tBackgroundNodeIds);
}

auto make_krino_wrapper_from_analysis_domain_mesh(const analysis::AnalysisDomainMesh& aAnalysisDomainMesh,
                                                  const std::set<std::string>& aFixedBlocks,
                                                  const tpik::VoidPhase& aVoidPhase,
                                                  const tpik::SnappingParameters aSnappingParameters) -> KrinoWrapper
{
    auto tKrinoMesh = tpik::read_and_setup_for_decomposition(aAnalysisDomainMesh.mFileName, aFixedBlocks);
    auto tLevelSet = tpik::get_level_set_fields(*tKrinoMesh);
    set_level_set_fields(*tKrinoMesh, tLevelSet, aAnalysisDomainMesh);
    return KrinoWrapper{std::move(tKrinoMesh), std::move(tLevelSet), aVoidPhase, aSnappingParameters};
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

auto compute_sensitivities(const stk::mesh::BulkData& aBulkData,
                           const std::vector<::krino::LS_Field>& aLevelSetFields,
                           const std::vector<tpik::BackgroundMeshNodeId>& aDesignDomainBackgroundNodes)
    -> tpik::SensitivityMap
{
    const auto tSensitivitiesFromKrino = tpik::get_krino_sensitivities(aBulkData, aLevelSetFields);
    const auto tSpatialDimension = aBulkData.mesh_meta_data().spatial_dimension();
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

}  // namespace detail

}  // namespace plato::geometry::extension
