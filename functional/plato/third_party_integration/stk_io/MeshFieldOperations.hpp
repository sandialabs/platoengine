#ifndef PLATO_THIRD_PARTY_INTEGRATION_STK_IO_MESHFIELDOPERATIONS
#define PLATO_THIRD_PARTY_INTEGRATION_STK_IO_MESHFIELDOPERATIONS

#include <numeric>
#include <ranges>
#include <stk_mesh/base/BulkData.hpp>
#include <stk_mesh/base/MetaData.hpp>

#include "plato/third_party_integration/stk_io/ReadUtilities.hpp"
#include "plato/utilities/ContainerHelpers.hpp"

namespace plato::third_party_integration::stk_io
{
/// @brief Given a range @a aNodalScalarField, whose size corresponds to the number of nodes in @a aBulk, computes the
/// nodal average of the field represented by the range.
/// @note The order of the elements in @a aNodalScalarField are assumed to be sorted in ascending order of global node
/// id.
[[nodiscard]] auto nodal_average(const std::ranges::random_access_range auto& aNodalScalarField,
                                 const stk::mesh::BulkData& aBulk) -> std::vector<double>;

namespace detail
{
/// @brief Given a @a aGlobalNodeID and list of all global node ids sorted in ascending order @a aSortedGlobalNodeIDs,
/// returns the index at which @a aGlobalNodeID appears in the @a aSortedGlobalNodeIDs.
/// @pre @a aSortedGlobalNodeIDs is sorted.
/// @pre @a aGlobalNodeID is an entry in @a aSortedGlobalNodeIDs.
[[nodiscard]] auto global_to_local_node_index(const std::vector<std::size_t>& aSortedGlobalNodeIDs,
                                              const std::size_t aGlobalNodeID) -> std::size_t;

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
                                                   detail::global_to_local_node_index(aGlobalNodeIDs, tGlobalID);
                                               return aCurrentAverage + aNodalScalarField[tLocalIndex];
                                           });
    return tNodalSum / static_cast<double>(tNumberOfNodes);
}
}  // namespace detail

auto nodal_average(const std::ranges::random_access_range auto& aNodalScalarField, const stk::mesh::BulkData& aBulkData)
    -> std::vector<double>
{
    const auto tGlobalNodeIDs = node_ids(aBulkData, aBulkData.mesh_meta_data().universal_part());
    assert(tGlobalNodeIDs.size() == static_cast<std::size_t>(aNodalScalarField.size()));

    auto tNodalAverage = utilities::reserved_container<std::vector<double>>(element_size(aBulkData));
    std::transform(aBulkData.begin_entities(stk::topology::ELEMENT_RANK),
                   aBulkData.end_entities(stk::topology::ELEMENT_RANK), std::back_inserter(tNodalAverage),
                   [&](const auto& aElement)
                   {
                       return detail::single_element_nodal_average(aElement.second, aNodalScalarField, tGlobalNodeIDs,
                                                                   aBulkData);
                   });
    return tNodalAverage;
}
}  // namespace plato::third_party_integration::stk_io

#endif
