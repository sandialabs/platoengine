#include "plato/mesh/MeshBlocks.hpp"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <iterator>
#include <optional>
#include <ranges>
#include <set>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

#include "plato/mesh/Mesh.hpp"
#include "plato/third_party_integration/common/BlockData.hpp"
#include "plato/third_party_integration/stk_io/BlockUtilities.hpp"
#include "plato/third_party_integration/stk_io/ReadUtilities.hpp"

namespace plato::mesh
{
namespace
{
using BlockDataIteratorType = std::vector<third_party_integration::common::BlockData>::const_iterator;

template <typename Predicate, typename FieldAccessor>
auto block_data_matching_predicate(const std::vector<third_party_integration::common::BlockData>& aBlockData,
                                   const Predicate& aPredicate,
                                   const FieldAccessor& aResultField)
    -> std::optional<std::invoke_result_t<FieldAccessor, const BlockDataIteratorType>>
{
    const auto tDataIterator = std::find_if(aBlockData.cbegin(), aBlockData.cend(), aPredicate);
    if (tDataIterator != aBlockData.cend())
    {
        return aResultField(tDataIterator);
    }
    return std::nullopt;
}
}  // namespace

MeshBlocks::MeshBlocks(Mesh aMeshBase) : Mesh{std::move(aMeshBase)} {}

auto MeshBlocks::blockOrdinal(std::string_view aBlockName) const -> std::optional<Mesh::BlockOrdinalType>
{
    return block_data_matching_predicate(
        blockData(), [aBlockName](const auto& aData) { return aData.mName == aBlockName; },
        [](const auto aIterator) { return aIterator->mMetaDataOrdinal; });
}

auto MeshBlocks::blockID(const std::string_view aBlockName) const -> std::optional<MeshBlocks::BlockIDType>
{
    return block_data_matching_predicate(
        blockData(), [aBlockName](const auto& aData) { return aData.mName == aBlockName; },
        [](const auto aIterator) { return aIterator->mID; });
}

auto MeshBlocks::blockID(const Mesh::BlockOrdinalType aBlockOrdinal) const -> std::optional<MeshBlocks::BlockIDType>
{
    return block_data_matching_predicate(
        blockData(), [aBlockOrdinal](const auto& aData) { return aData.mMetaDataOrdinal == aBlockOrdinal; },
        [](const auto aIterator) { return aIterator->mID; });
}

auto MeshBlocks::blockData() const -> std::vector<third_party_integration::common::BlockData>
{
    return third_party_integration::stk_io::block_data(bulkData());
}

auto MeshBlocks::nodeIDs(const BlockOrdinalType aBlockOrdinal) const -> std::vector<std::size_t>
{
    if (const auto tPart =
            third_party_integration::stk_io::part_with_block_meta_data_ordinal(bulkData(), aBlockOrdinal))
    {
        return third_party_integration::stk_io::node_ids(bulkData(), tPart.value().get());
    }
    return {};
}

auto MeshBlocks::elementIDs(const BlockOrdinalType aBlockOrdinal) const -> std::vector<std::size_t>
{
    if (const auto tPart =
            third_party_integration::stk_io::part_with_block_meta_data_ordinal(bulkData(), aBlockOrdinal))
    {
        return third_party_integration::stk_io::element_ids(bulkData(), tPart.value().get());
    }
    return {};
}

auto MeshBlocks::blockNames() const -> std::vector<std::string>
{
    const auto tBlockData = blockData();
    auto tBlockNames = std::vector<std::string>{};
    tBlockNames.reserve(tBlockNames.size());
    std::transform(tBlockData.cbegin(), tBlockData.cend(), std::back_inserter(tBlockNames),
                   [](const auto& aBlockDatum) { return aBlockDatum.mName; });
    return tBlockNames;
}

auto MeshBlocks::fixedBlockNames() const -> std::set<std::string>
{
    auto tBlockNames = fixedDomainBlocks() |
                       std::views::transform([](const auto& aFixedBlockPart) { return aFixedBlockPart.get().name(); });
    return {tBlockNames.begin(), tBlockNames.end()};
}

auto block_ids(const Mesh& aMesh, const std::vector<Mesh::BlockOrdinalType>& aBlockOrdinals)
    -> std::vector<MeshBlocks::BlockIDType>
{
    auto tBlockIDs = std::vector<MeshBlocks::BlockIDType>{};
    tBlockIDs.reserve(aBlockOrdinals.size());
    std::transform(aBlockOrdinals.cbegin(), aBlockOrdinals.cend(), std::back_inserter(tBlockIDs),
                   [&aMesh](const auto aBlockOrdinal)
                   {
                       const auto tBlockID = mesh::MeshBlocks{aMesh}.blockID(aBlockOrdinal);
                       assert(tBlockID.has_value());
                       return tBlockID.value();
                   });
    return tBlockIDs;
}

auto fixed_block_names(const analysis::AnalysisDomainMesh& aAnalysisDomainMesh) -> std::set<std::string>
{
    return MeshBlocks{Mesh{aAnalysisDomainMesh}}.fixedBlockNames();
}

}  // namespace plato::mesh
