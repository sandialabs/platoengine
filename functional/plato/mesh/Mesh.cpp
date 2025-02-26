#include "plato/mesh/Mesh.hpp"

#include <cassert>

#include "plato/analysis/AnalysisDomainMesh.hpp"
#include "plato/third_party_integration/stk_io/BlockUtilities.hpp"
#include "plato/third_party_integration/stk_io/ReadUtilities.hpp"

namespace plato::mesh
{
namespace
{
template <typename T, typename FieldFunction>
Mesh::BlockOrdinalType block_meta_data_ordinal(
    const T aBlockField,
    const FieldFunction& aFieldFunction,
    const std::vector<third_party_integration::common::BlockData>& aBlockData)
{
    const auto tBlockDataWithField = std::find_if(aBlockData.cbegin(), aBlockData.cend(),
                                                  [aBlockField, aFieldFunction](const auto& aBlockDatum)
                                                  { return aFieldFunction(aBlockDatum) == aBlockField; });
    assert(tBlockDataWithField != aBlockData.cend());
    return tBlockDataWithField->mMetaDataOrdinal;
}

auto block_ordinals_from_names(const std::shared_ptr<stk::mesh::BulkData>& aBulkData,
                               const std::set<std::string>& aBlockNames) -> std::vector<Mesh::BlockOrdinalType>
{
    namespace tpi = third_party_integration;

    if (!aBulkData)
    {
        return {};
    }

    const auto tNameField = [](const tpi::common::BlockData& aBlockData) { return aBlockData.mName; };

    const auto tBlockData = tpi::stk_io::block_data(*aBulkData);
    auto tBlockIDs = std::vector<Mesh::BlockOrdinalType>{};
    tBlockIDs.reserve(aBlockNames.size());
    std::transform(aBlockNames.cbegin(), aBlockNames.cend(), std::back_inserter(tBlockIDs),
                   [&tBlockData, &tNameField](const std::string& tBlockName)
                   { return block_meta_data_ordinal(tBlockName, tNameField, tBlockData); });
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

auto set_difference_block_ordinals(const std::shared_ptr<stk::mesh::BulkData>& aBulkData,
                                   const std::vector<Mesh::BlockOrdinalType>& aBlockOrdinals)
    -> std::vector<Mesh::BlockOrdinalType>
{
    if (!aBulkData)
    {
        return {};
    }
    const auto tAllBlockOrdinals = all_block_ordinals(*aBulkData);
    auto tDifferenceBlockOrdinals = std::vector<Mesh::BlockOrdinalType>{};
    std::set_difference(tAllBlockOrdinals.cbegin(), tAllBlockOrdinals.cend(), aBlockOrdinals.cbegin(),
                        aBlockOrdinals.cend(), std::back_inserter(tDifferenceBlockOrdinals));
    return tDifferenceBlockOrdinals;
}

auto fixed_block_ordinals_from_mesh_analysis(const std::shared_ptr<stk::mesh::BulkData>& aBulkData,
                                             const analysis::AnalysisDomainMesh& aAnalysisDomainMesh)
    -> std::vector<Mesh::BlockOrdinalType>
{
    namespace tpi = third_party_integration;

    if (!aBulkData)
    {
        return {};
    }

    const auto tIDField = [](const tpi::common::BlockData& aBlockData) { return aBlockData.mID; };

    const auto tBlockData = tpi::stk_io::block_data(*aBulkData);
    auto tDesignBlockIDs = std::vector<Mesh::BlockOrdinalType>{};
    tDesignBlockIDs.reserve(aAnalysisDomainMesh.mBlockScalarField.size());
    std::transform(aAnalysisDomainMesh.mBlockScalarField.cbegin(), aAnalysisDomainMesh.mBlockScalarField.cend(),
                   std::back_inserter(tDesignBlockIDs),
                   [&tBlockData, &tIDField](const auto& tBlockScalarField)
                   { return block_meta_data_ordinal(tBlockScalarField.first, tIDField, tBlockData); });

    return set_difference_block_ordinals(aBulkData, tDesignBlockIDs);
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

auto read_mesh_file(const std::filesystem::path& aFilePath) -> std::shared_ptr<stk::mesh::BulkData>
{
    if (!std::filesystem::exists(aFilePath))
    {
        return nullptr;
    }
    return third_party_integration::stk_io::read_mesh_bulk_data(aFilePath);
}

}  // namespace

/// @brief Loads a mesh from disk at the path @a aMeshName
Mesh::Mesh(const std::filesystem::path& aMeshName, const std::set<std::string>& aFixedBlockNames)
    : mFilePath{aMeshName},
      mBulk{read_mesh_file(aMeshName)},
      mFixedBlockOrdinals{block_ordinals_from_names(mBulk, aFixedBlockNames)},
      mDesignBlockOrdinals{set_difference_block_ordinals(mBulk, mFixedBlockOrdinals)}
{
}

Mesh::Mesh(const analysis::AnalysisDomainMesh& aAnalysisDomainMesh)
    : mFilePath{aAnalysisDomainMesh.mFileName},
      mBulk{read_mesh_file(mFilePath)},
      mFixedBlockOrdinals{fixed_block_ordinals_from_mesh_analysis(mBulk, aAnalysisDomainMesh)},
      mDesignBlockOrdinals{set_difference_block_ordinals(mBulk, mFixedBlockOrdinals)}
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

stk::mesh::BulkData& Mesh::bulkData()
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

auto Mesh::valid() const -> bool { return mBulk != nullptr; }

}  // namespace plato::mesh
