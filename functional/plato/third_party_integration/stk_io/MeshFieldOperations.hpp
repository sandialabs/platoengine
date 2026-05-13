#ifndef PLATO_THIRD_PARTY_INTEGRATION_STK_IO_MESHFIELDOPERATIONS
#define PLATO_THIRD_PARTY_INTEGRATION_STK_IO_MESHFIELDOPERATIONS

#include <numeric>
#include <ranges>
#include <stk_mesh/base/BulkData.hpp>
#include <stk_mesh/base/GetEntities.hpp>
#include <stk_mesh/base/MetaData.hpp>

#include "plato/third_party_integration/stk_io/ReadUtilities.hpp"
#include "plato/utilities/ContainerHelpers.hpp"
#include "plato/utilities/Zip.hpp"

namespace plato::third_party_integration::stk_io
{
/// @brief Given a range @a aNodalScalarField, whose size corresponds to the number of nodes in @a aBulk, this computes
/// the element-wise nodal average of the field represented by the range.
/// @pre The order of the elements in @a aNodalScalarField are assumed to be sorted in ascending order of global node
/// id.
/// @post The size of the returned vector will be equal to the total number of elements in @a aBulkData. The entries in
/// the returned vector will be sorted in ascending global element ID order.
[[nodiscard]] auto element_averaged_nodal_values(const std::ranges::random_access_range auto& aNodalScalarField,
                                                 const stk::mesh::BulkData& aBulkData) -> std::vector<double>;

/// @brief Given a range @a aNodalScalarField, whose size corresponds to the number of nodes the parts reference
/// vector @a aParts for a bulk data @a aBulk, compute the element-wise nodal average of the field.
[[nodiscard]] auto element_averaged_nodal_values(const std::ranges::random_access_range auto& aNodalScalarField,
                                                 const stk::mesh::BulkData& aBulkData,
                                                 const PartReferenceVector& aParts) -> std::vector<double>;

/// @brief Given a range @a aElementScalarField, whose size corresponds to the number of elements in @a aBulk, this
/// computes the projection of the element values to the nodes, using the nodal average to weight each contribution.
/// @note This operation is essentially the adjoint of element_averaged_nodal_value.
/// @pre The order of the elements in @a aElementScalarField are assumed to be sorted in ascending order of global
/// element id.
/// @post The size of the returned vector will be equal to the total number of nodes in @a aBulkData. The entries in the
/// returned vector will be sorted in ascending global node ID order.
[[nodiscard]] auto nodal_average_element_projection(const std::ranges::random_access_range auto& aElementScalarField,
                                                    const stk::mesh::BulkData& aBulkData) -> std::vector<double>;

/// @brief Given a range @a aElementScalarField, whose size corresponds to the number of elements in the parts reference
/// vector @a aParts for a bulk data @a aBulk, this computes the projection of the element values to the nodes, using
/// the nodal average to weight each contribution.
[[nodiscard]] auto nodal_average_element_projection(const std::ranges::random_access_range auto& aElementScalarField,
                                                    const stk::mesh::BulkData& aBulkData,
                                                    const PartReferenceVector& aParts) -> std::vector<double>;

namespace detail
{
/// @brief Given a @a aGlobalID and list of all global ids sorted in ascending order @a aSortedGlobalIDs,
/// returns the index at which @a aGlobalID appears in the @a aSortedGlobalIDs.
/// @pre @a aSortedGlobalIDs is sorted.
/// @pre @a aGlobalID is an entry in @a aSortedGlobalIDs.
[[nodiscard]] auto global_to_local_index(const std::vector<std::size_t>& aSortedGlobalIDs,
                                         const std::size_t aGlobalID) -> std::size_t;

[[nodiscard]] auto single_element_nodal_average(const stk::mesh::Entity& aElement,
                                                const std::ranges::random_access_range auto& aNodalScalarField,
                                                const std::vector<std::size_t>& aGlobalNodeIDs,
                                                const stk::mesh::BulkData& aBulkData)
{
    const auto tNumberOfNodes = aBulkData.num_nodes(aElement);
    const auto tNodalSum = std::accumulate(aBulkData.begin_nodes(aElement), aBulkData.end_nodes(aElement), 0.0,
                                           [&](const double aCurrentAverage, const stk::mesh::Entity& aNodeEntity)
                                           {
                                               const auto tGlobalID = aBulkData.entity_key(aNodeEntity).id();
                                               const auto tLocalIndex =
                                                   detail::global_to_local_index(aGlobalNodeIDs, tGlobalID);
                                               return aCurrentAverage + aNodalScalarField[tLocalIndex];
                                           });
    return tNodalSum / static_cast<double>(tNumberOfNodes);
}

/// @brief Return the element entities associated with the parts in @a aParts and the bulk data @a aBulkData
[[nodiscard]] auto element_entities(const stk::mesh::BulkData& aBulkData,
                                    const PartReferenceVector& aParts) -> stk::mesh::EntityVector;
}  // namespace detail

auto element_averaged_nodal_values(const std::ranges::random_access_range auto& aNodalScalarField,
                                   const stk::mesh::BulkData& aBulkData) -> std::vector<double>
{
    return element_averaged_nodal_values(aNodalScalarField, aBulkData, universal_part(aBulkData));
}

auto element_averaged_nodal_values(const std::ranges::random_access_range auto& aNodalScalarField,
                                   const stk::mesh::BulkData& aBulkData,
                                   const PartReferenceVector& aParts) -> std::vector<double>
{
    const auto tGlobalNodeIDs = node_ids(aBulkData, aParts);
    assert(tGlobalNodeIDs.size() == static_cast<std::size_t>(aNodalScalarField.size()));

    auto tNodalAverage = utilities::reserved_container<std::vector<double>>(element_size(aBulkData, aParts));
    const auto tElementEntities = detail::element_entities(aBulkData, aParts);
    std::transform(
        tElementEntities.begin(), tElementEntities.end(), std::back_inserter(tNodalAverage), [&](const auto& aElement)
        { return detail::single_element_nodal_average(aElement, aNodalScalarField, tGlobalNodeIDs, aBulkData); });
    return tNodalAverage;
}

auto nodal_average_element_projection(const std::ranges::random_access_range auto& aElementScalarField,
                                      const stk::mesh::BulkData& aBulkData) -> std::vector<double>
{
    return nodal_average_element_projection(aElementScalarField, aBulkData, universal_part(aBulkData));
}

auto nodal_average_element_projection(const std::ranges::random_access_range auto& aElementScalarField,
                                      const stk::mesh::BulkData& aBulkData,
                                      const PartReferenceVector& aParts) -> std::vector<double>
{
    const auto tElementEntities = detail::element_entities(aBulkData, aParts);
    assert(aElementScalarField.size() == tElementEntities.size());
    const auto tGlobalNodeIDs = node_ids(aBulkData, aParts);
    auto tNodalProjection = std::vector<double>(tGlobalNodeIDs.size(), 0.0);

    for (const auto [tValue, tElementEntity] : utilities::Zip{aElementScalarField, tElementEntities})
    {
        const auto tNumberOfNodes = aBulkData.num_nodes(tElementEntity);
        std::for_each(aBulkData.begin_nodes(tElementEntity), aBulkData.end_nodes(tElementEntity),
                      [&](const auto& aNodeEntity)
                      {
                          const auto tGlobalNodeID = aBulkData.entity_key(aNodeEntity).id();
                          const auto tLocalIndex = detail::global_to_local_index(tGlobalNodeIDs, tGlobalNodeID);
                          // Spurious clang-tidy warning: https://github.com/llvm/llvm-project/issues/99764
                          // NOLINTBEGIN(clang-analyzer-core.NullDereference)
                          tNodalProjection[tLocalIndex] += tValue / static_cast<double>(tNumberOfNodes);
                          // NOLINTEND(clang-analyzer-core.NullDereference)
                      });
    }
    return tNodalProjection;
}
}  // namespace plato::third_party_integration::stk_io

#endif
