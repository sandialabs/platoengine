#include "plato/mesh/MeshBlocks.hpp"

#include <cassert>

#include "plato/third_party_integration/stk_io/BlockUtilities.hpp"
#include "plato/third_party_integration/stk_io/ReadUtilities.hpp"

namespace plato::mesh
{
namespace
{
using BlockDataIteratorType = std::vector<third_party_integration::common::BlockData>::const_iterator;

template <typename Predicate, typename FieldAccessor>
auto blockDataMatchingPredicate(const std::vector<third_party_integration::common::BlockData>& aBlockData,
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
    return blockDataMatchingPredicate(
        blockData(), [aBlockName](const auto& aData) { return aData.mName == aBlockName; },
        [](const auto aIterator) { return aIterator->mMetaDataOrdinal; });
}

auto MeshBlocks::blockID(const std::string_view aBlockName) const -> std::optional<MeshBlocks::BlockIDType>
{
    return blockDataMatchingPredicate(
        blockData(), [aBlockName](const auto& aData) { return aData.mName == aBlockName; },
        [](const auto aIterator) { return aIterator->mID; });
}

auto MeshBlocks::blockID(const Mesh::BlockOrdinalType aBlockOrdinal) const -> std::optional<MeshBlocks::BlockIDType>
{
    return blockDataMatchingPredicate(
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
                   [](const auto& tBlockDatum) { return tBlockDatum.mName; });
    return tBlockNames;
}

auto block_ids(const Mesh& aMesh, const std::vector<Mesh::BlockOrdinalType>& aBlockOrdinals)
    -> std::vector<MeshBlocks::BlockIDType>
{
    auto tFixedBlockIDs = std::vector<MeshBlocks::BlockIDType>{};
    tFixedBlockIDs.reserve(aBlockOrdinals.size());
    std::transform(aBlockOrdinals.cbegin(), aBlockOrdinals.cend(), std::back_inserter(tFixedBlockIDs),
                   [&aMesh](const auto aBlockOrdinal)
                   {
                       const auto tBlockID = mesh::MeshBlocks{aMesh}.blockID(aBlockOrdinal);
                       assert(tBlockID.has_value());
                       return tBlockID.value();
                   });
    return tFixedBlockIDs;
}

}  // namespace plato::mesh
