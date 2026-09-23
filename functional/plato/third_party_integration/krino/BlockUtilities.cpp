#include "plato/third_party_integration/krino/BlockUtilities.hpp"

#include <Akri_MeshInterface.hpp>
#include <algorithm>
#include <iterator>
#include <set>
#include <stk_mesh/base/MetaData.hpp>
#include <stk_mesh/base/Types.hpp>
#include <stk_topology/topology_decl.hpp>
#include <string>
#include <vector>

namespace plato::third_party_integration::krino
{
auto all_block_parts(const ::krino::MeshInterface& aKrinoMesh) -> stk::mesh::PartVector
{
    auto tBlockParts = stk::mesh::PartVector{};
    std::ranges::copy_if(aKrinoMesh.meta_data().get_parts(), std::back_inserter(tBlockParts),
                         [](const auto* aPart)
                         {
                             return aPart->primary_entity_rank() == stk::topology::ELEMENT_RANK &&
                                    aPart->subsets().empty() && aPart->topology() != stk::topology::INVALID_TOPOLOGY &&
                                    !stk::mesh::is_auto_declared_part(*aPart);
                         });
    return tBlockParts;
}

auto all_blocks_except(const ::krino::MeshInterface& aKrinoMesh,
                       const std::set<std::string>& aExcludedBlocks) -> stk::mesh::PartVector
{
    auto tElementBlockParts = all_block_parts(aKrinoMesh);
    std::erase_if(tElementBlockParts,
                  [&aExcludedBlocks](const auto& aPart) { return aExcludedBlocks.contains(aPart->name()); });
    return tElementBlockParts;
}

}  // namespace plato::third_party_integration::krino
