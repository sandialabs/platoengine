#include "plato/third_party_integration/stk_io/BlockUtilities.hpp"

#include <stk_mesh/base/Bucket.hpp>
#include <stk_mesh/base/BulkData.hpp>
#include <stk_mesh/base/Comm.hpp>
#include <stk_mesh/base/ExodusTranslator.hpp>
#include <stk_mesh/base/GetEntities.hpp>
#include <stk_mesh/base/MetaData.hpp>
#include <stk_mesh/base/Part.hpp>
#include <stk_mesh/base/Selector.hpp>

namespace plato::third_party_integration::stk_io
{
namespace
{
std::size_t entity_size(const stk::mesh::BulkData& aBulkData,
                        const stk::mesh::Part& aPart,
                        const stk::topology::rank_t aEntityType)
{
    const auto tSelector = stk::mesh::Selector{aPart};
    auto tEntityCounts = std::vector<std::size_t>{};
    stk::mesh::comm_mesh_counts(aBulkData, tEntityCounts, &tSelector);
    return tEntityCounts.at(aEntityType);
}

std::vector<std::size_t> entity_ids(const stk::mesh::BulkData& aBulkData,
                                    const stk::mesh::Part& aPart,
                                    const stk::topology::rank_t aEntityType)
{
    auto tEntities = std::vector<stk::mesh::Entity>{};
    stk::mesh::get_selected_entities(aPart, aBulkData.buckets(aEntityType), tEntities);
    auto tEntityIDs = std::vector<std::size_t>{};
    tEntityIDs.reserve(tEntities.size());
    std::transform(tEntities.cbegin(), tEntities.cend(), std::back_inserter(tEntityIDs),
                   [&aBulkData](const auto& tEntity) { return aBulkData.identifier(tEntity); });
    return tEntityIDs;
}

}  // namespace

unsigned int block_size(const stk::mesh::BulkData& aBulk) { return element_blocks_parts(aBulk).size(); }

std::vector<common::BlockData> block_data(const stk::mesh::BulkData& aBulk)
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

std::size_t element_size(const stk::mesh::BulkData& aBulkData, const stk::mesh::Part& aPart)
{
    return entity_size(aBulkData, aPart, stk::topology::ELEM_RANK);
}

std::size_t node_size(const stk::mesh::BulkData& aBulkData, const stk::mesh::Part& aPart)
{
    return entity_size(aBulkData, aPart, stk::topology::NODE_RANK);
}

std::vector<std::size_t> node_ids(const stk::mesh::BulkData& aBulkData, const stk::mesh::Part& aPart)
{
    return entity_ids(aBulkData, aPart, stk::topology::NODE_RANK);
}

std::vector<std::size_t> element_ids(const stk::mesh::BulkData& aBulkData, const stk::mesh::Part& aPart)
{
    return entity_ids(aBulkData, aPart, stk::topology::ELEM_RANK);
}

auto element_blocks_parts(const stk::mesh::BulkData& aBulkData) -> stk::mesh::PartVector
{
    const auto tExodusTranslator = stk::mesh::ExodusTranslator{aBulkData};
    return tExodusTranslator.get_element_block_parts();
}

}  // namespace plato::third_party_integration::stk_io
