#include "plato/third_party_integration/krino/BackgroundMeshUtilities.hpp"

#include <Akri_CDFEM_Support.hpp>
#include <algorithm>
#include <iterator>
#include <stk_mesh/base/GetEntities.hpp>
#include <stk_mesh/base/Selector.hpp>
#include <stk_mesh/base/Types.hpp>
#include <stk_topology/topology_decl.hpp>
#include <unordered_map>
#include <utility>
#include <vector>

#include "plato/third_party_integration/krino/BackgroundMesh.hpp"
#include "plato/third_party_integration/krino/Utilities.hpp"
#include "plato/utilities/ContainerHelpers.hpp"
#include "plato/utilities/ReduceUtilities.hpp"

namespace plato::third_party_integration::krino
{
auto background_node_ids(const BackgroundMesh& aBackgroundMesh) -> std::vector<stk::mesh::EntityId>
{
    const auto tBackgroundNodes = node_entities_in_mesh(aBackgroundMesh);
    return utilities::unique_vector_gather(detail::get_ids_from_entities(tBackgroundNodes, aBackgroundMesh.bulkData()),
                                           retrieve_mpi_communicator_from_krino());
}

auto get_level_set_values(const BackgroundMesh& aBackgroundMesh) -> std::unordered_map<stk::mesh::EntityId, double>
{
    const auto tNodes = node_entities_in_mesh(aBackgroundMesh);

    auto tLevelSetValues =
        utilities::reserved_container<std::unordered_map<stk::mesh::EntityId, double>>(tNodes.size());

    std::ranges::transform(tNodes, std::inserter(tLevelSetValues, tLevelSetValues.begin()),
                           [&aBackgroundMesh](const auto& aNode)
                           {
                               const auto tId = aBackgroundMesh.bulkData().entity_key(aNode).id();
                               return std::make_pair(tId, level_set_value(aBackgroundMesh.levelSetFields(), aNode));
                           });

    return utilities::reduce_map(tLevelSetValues, retrieve_mpi_communicator_from_krino());
}

auto node_entities_in_mesh(const BackgroundMesh& aBackgroundMesh) -> stk::mesh::EntityVector
{
    const ::krino::CDFEM_Support& tCdfemSupport = ::krino::CDFEM_Support::get(aBackgroundMesh.metaData());
    const stk::mesh::Selector tSelector =
        stk::mesh::selectField(aBackgroundMesh.levelSetFields().front().isovar) & !tCdfemSupport.get_child_node_part();
    stk::mesh::EntityVector tNodes;
    stk::mesh::get_selected_entities(tSelector, aBackgroundMesh.bulkData().buckets(stk::topology::NODE_RANK), tNodes,
                                     sort_by_global_id());
    return tNodes;
}

}  // namespace plato::third_party_integration::krino
