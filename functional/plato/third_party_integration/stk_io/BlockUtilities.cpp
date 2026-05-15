#include "plato/third_party_integration/stk_io/BlockUtilities.hpp"

#include <algorithm>
#include <cstddef>
#include <functional>
#include <iterator>
#include <optional>
#include <stk_mesh/base/Bucket.hpp>
#include <stk_mesh/base/BulkData.hpp>
#include <stk_mesh/base/Comm.hpp>
#include <stk_mesh/base/ExodusTranslator.hpp>
#include <stk_mesh/base/GetEntities.hpp>
#include <stk_mesh/base/MetaData.hpp>
#include <stk_mesh/base/Part.hpp>
#include <stk_mesh/base/Selector.hpp>
#include <stk_topology/topology_decl.hpp>
#include <vector>

#include "plato/third_party_integration/common/BlockData.hpp"

namespace plato::third_party_integration::stk_io
{
namespace
{
auto entity_size(const stk::mesh::BulkData& aBulkData,
                 const stk::mesh::Part& aPart,
                 const stk::topology::rank_t aEntityType) -> std::size_t
{
    const auto tSelector = stk::mesh::Selector{aPart};
    auto tEntityCounts = std::vector<std::size_t>{};
    stk::mesh::comm_mesh_counts(aBulkData, tEntityCounts, &tSelector);
    return tEntityCounts.at(aEntityType);
}

}  // namespace

auto block_size(const stk::mesh::BulkData& aBulk) -> unsigned int { return element_blocks_parts(aBulk).size(); }

auto block_data(const stk::mesh::BulkData& aBulk) -> std::vector<common::BlockData>
{
    const auto tParts = element_blocks_parts(aBulk);
    auto tBlockData = std::vector<common::BlockData>{};
    tBlockData.reserve(tParts.size());
    std::transform(tParts.cbegin(), tParts.cend(), std::back_inserter(tBlockData),
                   [](const auto& aPart) {
                       return common::BlockData{aPart->id(), aPart->mesh_meta_data_ordinal(), aPart->name()};
                   });
    std::sort(tBlockData.begin(), tBlockData.end(),
              [](const auto& tBlockDataLeft, const auto& tBlockDataRight)
              { return tBlockDataLeft.mID < tBlockDataRight.mID; });
    return tBlockData;
}

auto part_with_block_meta_data_ordinal(const stk::mesh::BulkData& aBulkData,
                                       const common::BlockData::BlockOrdinalType aBlockOrdinal) -> OptionalPartReference
{
    if (aBulkData.mesh_meta_data().is_valid_part_ordinal(aBlockOrdinal))
    {
        const auto& tPart = aBulkData.mesh_meta_data().get_part(aBlockOrdinal);
        return std::make_optional(std::cref(tPart));
    }
    else
    {
        return std::nullopt;
    }
}

auto element_size(const stk::mesh::BulkData& aBulkData, const stk::mesh::Part& aPart) -> std::size_t
{
    return entity_size(aBulkData, aPart, stk::topology::ELEM_RANK);
}

auto node_size(const stk::mesh::BulkData& aBulkData, const stk::mesh::Part& aPart) -> std::size_t
{
    return entity_size(aBulkData, aPart, stk::topology::NODE_RANK);
}

auto element_blocks_parts(const stk::mesh::BulkData& aBulkData) -> stk::mesh::PartVector
{
    const auto tExodusTranslator = stk::mesh::ExodusTranslator{aBulkData};
    return tExodusTranslator.get_element_block_parts();
}

}  // namespace plato::third_party_integration::stk_io
