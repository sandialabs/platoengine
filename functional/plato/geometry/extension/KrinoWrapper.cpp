#include "plato/geometry/extension/KrinoWrapper.hpp"

#include <boost/mpi/collectives.hpp>
#include <boost/mpi/communicator.hpp>
#include <boost/serialization/unordered_map.hpp>
#include <boost/serialization/vector.hpp>
#include <cstddef>
#include <stk_mesh/base/Entity.hpp>
#include <stk_mesh/base/Types.hpp>
#include <stk_util/environment/EnvData.hpp>  //get stk mpi env
#include <string_view>
#include <unordered_map>
#include <vector>

#include "plato/analysis/AnalysisDomainMesh.hpp"
#include "plato/analysis/AnalysisDomainMeshSequentialView.hpp"
#include "plato/mesh/EntityRetrieval.hpp"
#include "plato/mesh/Mesh.hpp"
#include "plato/third_party_integration/krino/Utilities.hpp"
#include "plato/third_party_integration/stk_io/ReadUtilities.hpp"  //spatial_dimensions
#include "plato/utilities/Enumerate.hpp"
#include "plato/utilities/MultiVectorView.hpp"
#include "plato/utilities/NamedType.hpp"
#include "plato/utilities/TransformIf.hpp"
#include "plato/utilities/Zip.hpp"

namespace plato::geometry::extension
{
namespace
{
namespace tpik = third_party_integration::krino;
constexpr auto kXComponent = utilities::ComponentIndex{0};
constexpr auto kYComponent = utilities::ComponentIndex{1};
constexpr auto kZComponent = utilities::ComponentIndex{2};

[[nodiscard]] auto row_vector_to_vector3(const std::vector<double>& aRowVector,
                                         const utilities::VectorIndex aVectorIndex,
                                         const std::size_t aDimensions) -> third_party_integration::common::Vector3
{
    const auto tRowVectorView = utilities::make_multi_vector_view(aRowVector, aDimensions);
    return third_party_integration::common::Vector3{
        tRowVectorView(aVectorIndex, kXComponent), tRowVectorView(aVectorIndex, kYComponent),
        aDimensions == 3U ? tRowVectorView(aVectorIndex, kZComponent) : 0.0};
}

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

[[nodiscard]] auto reduce_level_set_values_map(const std::unordered_map<stk::mesh::EntityId, double>& aLevelSetMap)
    -> std::unordered_map<stk::mesh::EntityId, double>
{
    const auto tCommunicator = boost::mpi::communicator(
        reinterpret_cast<ompi_communicator_t*>(stk::EnvData::instance().m_parallelComm), boost::mpi::comm_duplicate);
    constexpr int tRootRank = 0;
    std::vector<std::unordered_map<stk::mesh::EntityId, double>> tGatheredMaps;
    boost::mpi::gather(tCommunicator, aLevelSetMap, tGatheredMaps, tRootRank);

    std::unordered_map<stk::mesh::EntityId, double> tRootLevelSetMap;
    if (tCommunicator.rank() == tRootRank)
    {
        for (const auto& tMap : tGatheredMaps)
        {
            for (const auto& [tEntityId, tLevelSetValue] : tMap)
            {
                tRootLevelSetMap[tEntityId] = tLevelSetValue;
            }
        }
    }
    boost::mpi::broadcast(tCommunicator, tRootLevelSetMap, tRootRank);
    return tRootLevelSetMap;
}

[[nodiscard]] auto cut_mesh_compute_sensitivities(
    stk::mesh::BulkData& aBulkData,
    const std::vector<::krino::LS_Field>& aLevelSetFields,
    const std::vector<tpik::BackgroundMeshNodeId>& aDesignDomainBackgroundNodes) -> tpik::SensitivityMap
{
    tpik::cut_mesh(aBulkData, aLevelSetFields);
    return detail::compute_sensitivities(aBulkData, aLevelSetFields, aDesignDomainBackgroundNodes);
}

}  // namespace

KrinoWrapper::KrinoWrapper(std::unique_ptr<::krino::MeshInterface> aKrinoMeshInterface,
                           std::vector<::krino::LS_Field> aLevelSetField,
                           std::optional<std::vector<tpik::BackgroundMeshNodeId>> aBackgroundDesignIDs)
    : mKrinoMesh(std::move(aKrinoMeshInterface)),
      mLevelSetFields(std::move(aLevelSetField)),
      mNumberOfDesignDomainBackgroundNodes(
          aBackgroundDesignIDs.value_or(tpik::background_node_ids(*mKrinoMesh, mLevelSetFields)).size()),
      mSensitivityMap(cut_mesh_compute_sensitivities(
          mKrinoMesh->bulk_data(),
          mLevelSetFields,
          std::move(aBackgroundDesignIDs).value_or(tpik::background_node_ids(*mKrinoMesh, mLevelSetFields))))

{
}

void KrinoWrapper::writeCutMesh(const std::filesystem::path& aFileName, const tpik::VoidPhase aVoidPhase) const
{
    tpik::write_mesh(mKrinoMesh->bulk_data(), aFileName, aVoidPhase);
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
    aResultVectorView(utilities::VectorIndex{aParentIndex.mValue}, kScalarViewComponent) +=
        third_party_integration::common::dot(
            row_vector_to_vector3(aRowVector, utilities::VectorIndex{aCutMeshIndex.mValue}, aSpatialDimension),
            aSensitivity) *
        aMultiplicityMultiplier;
};

const auto kAdjointJacobianImpl = [](utilities::MultiVectorView<std::vector<double>>& aResultVectorView,
                                     const std::vector<double>& aRowVector,
                                     const ParentIndex aParentIndex,
                                     const CutMeshIndex aCutMeshIndex,
                                     const double aMultiplicityMultiplier,
                                     const third_party_integration::common::Vector3& aSensitivity,
                                     const unsigned int aSpatialDimension)
{
    aResultVectorView(utilities::VectorIndex{aCutMeshIndex.mValue}, kXComponent) +=
        aRowVector[aParentIndex.mValue] * aSensitivity.x * aMultiplicityMultiplier;
    aResultVectorView(utilities::VectorIndex{aCutMeshIndex.mValue}, kYComponent) +=
        aRowVector[aParentIndex.mValue] * aSensitivity.y * aMultiplicityMultiplier;
    if (aSpatialDimension == 3U)
    {
        aResultVectorView(utilities::VectorIndex{aCutMeshIndex.mValue}, kZComponent) +=
            aRowVector[aParentIndex.mValue] * aSensitivity.z * aMultiplicityMultiplier;
    }
};

[[nodiscard]] auto reduce_vector(const std::vector<double>& aVector) -> std::vector<double>
{
    std::vector<double> tGlobal(aVector.size(), 0.0);
    constexpr int tRootRank = 0;
    const auto tCommunicator = boost::mpi::communicator(
        reinterpret_cast<ompi_communicator_t*>(stk::EnvData::instance().m_parallelComm), boost::mpi::comm_duplicate);

    boost::mpi::reduce(tCommunicator, aVector, tGlobal, std::plus<double>(), tRootRank);
    boost::mpi::broadcast(tCommunicator, tGlobal, tRootRank);

    return tGlobal;
}

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
            std::cout << "CutMesh ID: " << tCutMeshId << " has multiplier: " << tMultiplicityMultiplier << std::endl;
            const auto& tLevelSetJacobianColumn = tSensitivityMapAtCutMeshIdIterator->second;

            for (const auto& [tParentId, tSensitivity, tLocalParentIndex] : utilities::Zip(
                     tLevelSetJacobianColumn.mBackgroundMeshNodeIDs, tLevelSetJacobianColumn.mNodalSensitivities,
                     tLevelSetJacobianColumn.mDesignDomainLocalIndex))
            {
                std::cout << "Cutmesh id: " << tCutMeshId << " with index " << tIndex
                          << " has local Parent index:  " << tLocalParentIndex << " and parent id: " << tParentId
                          << " and sensitivity: " << tSensitivity.x << ", " << tSensitivity.y << ", " << tSensitivity.z
                          << std::endl;
                aApplyFunction(tRowVectorMatrixProductView, aRowVector, ParentIndex{tLocalParentIndex},
                               CutMeshIndex{tIndex}, tMultiplicityMultiplier, tSensitivity, tSpatialDimensions);
            }
        }
    }

    return reduce_vector(tRowVectorMatrixProduct);
}

}  // namespace

auto KrinoWrapper::rowVectorJacobianProduct(const std::vector<double>& aCutMeshRowVector,
                                            const tpik::VoidPhase aVoidPhase) const -> std::vector<double>
{
    const auto tViewDimension = 1U;
    const auto tResultSize = mNumberOfDesignDomainBackgroundNodes;
    const auto tBackgroundNodeIds = tpik::background_node_ids(*mKrinoMesh, mLevelSetFields);

    return transformSensitivityMap(aCutMeshRowVector, mSensitivityMap, *mKrinoMesh, aVoidPhase, ResultSize{tResultSize},
                                   ResultViewDimensionality{tViewDimension}, kJacobianImpl);
}

auto KrinoWrapper::rowVectorAdjointJacobianProduct(const std::vector<double>& aBackgroundMeshRowVector,
                                                   const tpik::VoidPhase aVoidPhase) const -> std::vector<double>
{
    const auto tViewDimension = third_party_integration::stk_io::spatial_dimensions(mKrinoMesh->bulk_data());
    const auto tResultSize = tpik::cut_mesh_node_ids(*mKrinoMesh, aVoidPhase).size() * tViewDimension;
    return transformSensitivityMap(aBackgroundMeshRowVector, mSensitivityMap, *mKrinoMesh, aVoidPhase,
                                   ResultSize{tResultSize}, ResultViewDimensionality{tViewDimension},
                                   kAdjointJacobianImpl);
}

namespace
{

[[nodiscard]] auto down_select_to_design_domain(
    const std::unordered_map<tpik::BackgroundMeshNodeId, double>& aLevelSetValuesMap,
    const std::vector<tpik::BackgroundMeshNodeId>& aBackgroundDesignIDs)
{
    const auto tFoundCondition = [&aLevelSetValuesMap](const auto aDesignDomainId) -> bool
    { return aLevelSetValuesMap.find(aDesignDomainId) != aLevelSetValuesMap.end(); };

    std::vector<double> tLevelSetValues;
    tLevelSetValues.reserve(aBackgroundDesignIDs.size());
    utilities::transform_if(
        aBackgroundDesignIDs, std::back_inserter(tLevelSetValues),
        [&aLevelSetValuesMap](const auto aBackgroundId) { return aLevelSetValuesMap.at(aBackgroundId); },
        tFoundCondition);

    return tLevelSetValues;
}

}  // namespace

auto make_initial_guess_from_level_set_primitives(
    const std::filesystem::path& aFileName,
    const tpik::LevelSetPrimitives& aLevelSetPrimitives,
    const std::optional<std::vector<tpik::BackgroundMeshNodeId>>& aBackgroundDesignIDs) -> std::vector<double>
{
    auto tKrinoMesh = tpik::read_and_setup_for_decomposition(aFileName);
    auto tLevelSetFields = tpik::make_level_set_field_from_primitives(aLevelSetPrimitives, tKrinoMesh->bulk_data());
    const auto tLevelSetValuesMap =
        reduce_level_set_values_map(tpik::get_level_set_values(*tKrinoMesh, tLevelSetFields));

    const auto tBackgroundNodeIds =
        aBackgroundDesignIDs.value_or(tpik::background_node_ids(*tKrinoMesh, tLevelSetFields));

    return down_select_to_design_domain(tLevelSetValuesMap, tBackgroundNodeIds);
}

auto make_krino_wrapper_from_analysis_domain_mesh(const analysis::AnalysisDomainMesh& aAnalysisDomainMesh,
                                                  const double aFixedBlockLevelSetValue) -> KrinoWrapper
{
    auto tKrinoMesh = tpik::read_and_setup_for_decomposition(aAnalysisDomainMesh.mFileName);
    auto tLevelSet = tpik::make_level_set_field_from_fixed_value(*tKrinoMesh, aFixedBlockLevelSetValue);
    set_level_set_fields(*tKrinoMesh, tLevelSet, aAnalysisDomainMesh);
    const auto tDesignDomainNodeIds = mesh::EntityRetrieval{mesh::Mesh{aAnalysisDomainMesh}}.designDomainNodeIDs();
    return KrinoWrapper{std::move(tKrinoMesh), std::move(tLevelSet), tDesignDomainNodeIds};
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
    std::vector<tpik::BackgroundMeshNodeId>& aParentNodeIds,
    std::vector<third_party_integration::common::Vector3>& aSensitivities,
    const std::vector<tpik::BackgroundMeshNodeId>& aDesignDomainBackgroundNodes)
    -> std::optional<tpik::LevelSetJacobianColumn>
{
    assert(aParentNodeIds.size() == 2U);
    assert(aSensitivities.size() == 2U);

    tpik::LevelSetJacobianColumn tLevelSetJacobianColumn;
    add_if_found(tLevelSetJacobianColumn, aParentNodeIds.front(), aSensitivities.front(), aDesignDomainBackgroundNodes);
    add_if_found(tLevelSetJacobianColumn, aParentNodeIds.back(), aSensitivities.back(), aDesignDomainBackgroundNodes);

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
    const ::krino::FieldRef tCoordsField = aBulkData.mesh_meta_data().coordinate_field();
    const auto tChildNodeStencils = tpik::get_child_node_stencils(aBulkData);
    const auto tSpatialDimension = aBulkData.mesh_meta_data().spatial_dimension();

    auto tSensitivityMap = tpik::SensitivityMap{};
    tSensitivityMap.reserve(tChildNodeStencils.size());
    for (auto& tStencil : tChildNodeStencils)
    {
        auto tParentNodeIds = tpik::parent_node_ids_from_parent_nodes(aBulkData, tStencil.parentNodes);
        auto tSensitivity = tpik::level_set_coordinate_sensitivity(
            tpik::CoordinateFieldReference{tCoordsField}, tpik::LevelSetFieldReference{aLevelSetFields.front().isovar},
            tStencil.parentNodes, tSpatialDimension);

        if (auto tLevelSetJacobianColumn =
                make_level_set_jacobian_column(tParentNodeIds, tSensitivity, aDesignDomainBackgroundNodes);
            tLevelSetJacobianColumn.has_value())
        {
            tSensitivityMap[aBulkData.identifier(tStencil.childNode)] = std::move(tLevelSetJacobianColumn).value();
        }
    }

    return tSensitivityMap;
}

}  // namespace detail

}  // namespace plato::geometry::extension
