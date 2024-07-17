#include "plato/third_party_integration/stk_io/BlockUtilities.hpp"

#include <stk_mesh/base/BulkData.hpp>
#include <stk_mesh/base/Comm.hpp>
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
}  // namespace

unsigned int block_size(const stk::mesh::BulkData& aBulk) { return aBulk.mesh_meta_data().get_mesh_parts().size(); }

std::vector<common::BlockData> block_data(const stk::mesh::BulkData& aBulk)
{
    const auto& tParts = aBulk.mesh_meta_data().get_mesh_parts();
    auto tBlockData = std::vector<common::BlockData>{};
    tBlockData.reserve(tParts.size());
    std::transform(tParts.cbegin(), tParts.cend(), std::back_inserter(tBlockData),
                   [](const auto& aPart) {
                       return common::BlockData{/*.mID=*/aPart->id(), /*.mName=*/aPart->name()};
                   });
    std::sort(tBlockData.begin(), tBlockData.end(),
              [](const auto& tBlockDataLeft, const auto& tBlockDataRight)
              { return tBlockDataLeft.mID < tBlockDataRight.mID; });
    return tBlockData;
}

auto part_with_block_name(const stk::mesh::BulkData& aBulkData, const std::string_view aBlockName)
    -> std::optional<std::reference_wrapper<stk::mesh::Part>>
{
    auto tPart = aBulkData.mesh_meta_data().get_part(std::string{aBlockName});
    if (tPart)
    {
        return std::make_optional(std::ref(*tPart));
    }
    else
    {
        return std::nullopt;
    }
    // stk::tools::extract_blocks(aBulkData, stk::mesh::BulkData & newBulk, const int& blockNames)
}

std::size_t element_size(const stk::mesh::BulkData& aBulkData, const stk::mesh::Part& aPart)
{
    return entity_size(aBulkData, aPart, stk::topology::ELEM_RANK);
}

std::size_t node_size(const stk::mesh::BulkData& aBulkData, const stk::mesh::Part& aPart)
{
    return entity_size(aBulkData, aPart, stk::topology::NODE_RANK);
}

}  // namespace plato::third_party_integration::stk_io
