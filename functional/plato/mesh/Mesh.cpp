#include "plato/mesh/Mesh.hpp"

#include <cassert>

#include "plato/mesh/MeshDesignVariables.hpp"
#include "plato/third_party_integration/stk_io/BlockUtilities.hpp"
#include "plato/third_party_integration/stk_io/Utilities.hpp"

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

std::vector<Mesh::BlockOrdinalType> block_ordinals_from_names(const stk::mesh::BulkData& aBulkData,
                                                              const std::set<std::string>& aBlockNames)
{
    namespace tpi = third_party_integration;

    const auto tNameField = [](const tpi::common::BlockData& aBlockData) { return aBlockData.mName; };

    const auto tBlockData = tpi::stk_io::block_data(aBulkData);
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

std::vector<Mesh::BlockOrdinalType> set_difference_block_ordinals(
    const stk::mesh::BulkData& aBulkData, const std::vector<Mesh::BlockOrdinalType>& aBlockOrdinals)
{
    const auto tAllBlockOrdinals = all_block_ordinals(aBulkData);
    auto tDifferenceBlockOrdinals = std::vector<Mesh::BlockOrdinalType>{};
    std::set_difference(tAllBlockOrdinals.cbegin(), tAllBlockOrdinals.cend(), aBlockOrdinals.cbegin(),
                        aBlockOrdinals.cend(), std::back_inserter(tDifferenceBlockOrdinals));
    return tDifferenceBlockOrdinals;
}

std::vector<Mesh::BlockOrdinalType> fixed_block_ordinals_from_mesh_design_variables(
    const stk::mesh::BulkData& aBulkData, const MeshDesignVariables& aMeshDesignVariables)
{
    namespace tpi = third_party_integration;

    const auto tIDField = [](const tpi::common::BlockData& aBlockData) { return aBlockData.mID; };

    const auto tBlockData = tpi::stk_io::block_data(aBulkData);
    auto tDesignBlockIDs = std::vector<Mesh::BlockOrdinalType>{};
    tDesignBlockIDs.reserve(aMeshDesignVariables.mBlockScalarField.size());
    std::transform(aMeshDesignVariables.mBlockScalarField.cbegin(), aMeshDesignVariables.mBlockScalarField.cend(),
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
}  // namespace

/// @brief Loads a mesh from disk at the path @a aMeshName
Mesh::Mesh(const std::filesystem::path& aMeshName, const std::set<std::string>& aFixedBlockNames)
    : mFilePath{aMeshName},
      mBulk{third_party_integration::stk_io::read_mesh_bulk_data(aMeshName)},
      mFixedBlockOrdinals{block_ordinals_from_names(*mBulk, aFixedBlockNames)},
      mDesignBlockOrdinals{set_difference_block_ordinals(*mBulk, mFixedBlockOrdinals)}
{
}

Mesh::Mesh(const MeshDesignVariables& aMeshDesignVariables)
    : mFilePath{aMeshDesignVariables.mFileName},
      mBulk{third_party_integration::stk_io::read_mesh_bulk_data(mFilePath)},
      mFixedBlockOrdinals{fixed_block_ordinals_from_mesh_design_variables(*mBulk, aMeshDesignVariables)},
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
