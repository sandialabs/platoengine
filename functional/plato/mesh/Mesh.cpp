#include "plato/mesh/Mesh.hpp"

#include "plato/third_party_integration/stk_io/BlockUtilities.hpp"
#include "plato/third_party_integration/stk_io/Utilities.hpp"

namespace plato::mesh
{
namespace
{

Mesh::BlockOrdinalType block_meta_data_ordinal(
    const std::string_view aBlockName, const std::vector<third_party_integration::common::BlockData>& aBlockData)
{
    const auto tBlockDataWithName =
        std::find_if(aBlockData.cbegin(), aBlockData.cend(),
                     [aBlockName](const auto& aBlockDatum) { return aBlockDatum.mName == aBlockName; });
    assert(tBlockDataWithName != aBlockData.cend());
    return tBlockDataWithName->mMetaDataOrdinal;
}

std::vector<Mesh::BlockOrdinalType> block_ordinals_from_names(const stk::mesh::BulkData& aBulkData,
                                                              const std::vector<std::string>& aBlockNames)
{
    namespace tpi = third_party_integration;

    const auto tBlockData = tpi::stk_io::block_data(aBulkData);
    auto tBlockIDs = std::vector<Mesh::BlockOrdinalType>{};
    tBlockIDs.reserve(aBlockNames.size());
    std::transform(aBlockNames.cbegin(), aBlockNames.cend(), std::back_inserter(tBlockIDs),
                   [&tBlockData](const std::string& tBlockName)
                   { return block_meta_data_ordinal(tBlockName, tBlockData); });
    return tBlockIDs;
}

std::vector<Mesh::BlockOrdinalType> all_block_ordinals(const stk::mesh::BulkData& aBulkData)
{
    const auto tBlockData = third_party_integration::stk_io::block_data(aBulkData);
    auto tBlockIDs = std::vector<Mesh::BlockOrdinalType>{};
    tBlockIDs.reserve(tBlockData.size());
    std::transform(tBlockData.cbegin(), tBlockData.cend(), std::back_inserter(tBlockIDs),
                   [](const auto& tBlockDatum) { return tBlockDatum.mMetaDataOrdinal; });
    return tBlockIDs;
}

std::vector<Mesh::BlockOrdinalType> set_difference_block_ordinals(
    const stk::mesh::BulkData& aBulkData, const std::vector<Mesh::BlockOrdinalType>& aBlockOrdinals)
{
    const auto tAllBlockOrdinals = all_block_ordinals(aBulkData);
    auto tDifferenceBlockOrdinals = std::vector<Mesh::BlockOrdinalType>{};
    std::set_difference(tAllBlockOrdinals.cbegin(), tAllBlockOrdinals.cend(), aBlockOrdinals.cbegin(),
                        aBlockOrdinals.cend(), std::back_inserter(tDifferenceBlockOrdinals));
    return tDifferenceBlockOrdinals;
}

Mesh::PartReferenceVector parts_from_block_ordinals(const stk::mesh::BulkData& aBulkData,
                                                    const std::vector<Mesh::BlockOrdinalType>& aBlockOrdinals)
{
    namespace tpi = third_party_integration;

    auto tParts = Mesh::PartReferenceVector{};
    tParts.reserve(aBlockOrdinals.size());
    std::transform(aBlockOrdinals.cbegin(), aBlockOrdinals.cend(), std::back_inserter(tParts),
                   [&aBulkData](const auto aBlockOrdinal)
                   {
                       auto tPart = tpi::stk_io::part_with_block_meta_data_ordinal(aBulkData, aBlockOrdinal);
                       assert(tPart);
                       return std::move(tPart).value();
                   });
    return tParts;
}
}  // namespace

/// @brief Loads a mesh from disk at the path @a aMeshName
Mesh::Mesh(const std::filesystem::path& aMeshName, const std::vector<std::string>& aFixedBlockNames)
    : mFilePath{aMeshName},
      mBulk{third_party_integration::stk_io::read_mesh_bulk_data(aMeshName)},
      mFixedBlockOrdinals{block_ordinals_from_names(*mBulk, aFixedBlockNames)},
      mDesignBlockOrdinals{set_difference_block_ordinals(*mBulk, mFixedBlockOrdinals)}
{
}

const std::filesystem::path& Mesh::filePath() const { return mFilePath; }

const std::vector<Mesh::BlockOrdinalType>& Mesh::fixedBlockOrdinals() const { return mFixedBlockOrdinals; }

const std::vector<Mesh::BlockOrdinalType>& Mesh::designBlockOrdinals() const { return mDesignBlockOrdinals; }

const stk::mesh::BulkData& Mesh::bulkData() const
{
    assert(mBulk);
    return *mBulk;
}

Mesh::PartReferenceVector Mesh::fixedDomainBlocks() const
{
    return parts_from_block_ordinals(bulkData(), mFixedBlockOrdinals);
}

Mesh::PartReferenceVector Mesh::designDomainBlocks() const
{
    return parts_from_block_ordinals(bulkData(), mDesignBlockOrdinals);
}

}  // namespace plato::mesh
