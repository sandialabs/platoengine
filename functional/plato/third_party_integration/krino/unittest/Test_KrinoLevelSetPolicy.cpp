#include <gtest/gtest.h>

#include <stk_util/environment/EnvData.hpp>

#include "plato/test_utilities/TestContext.hpp"
#include "plato/third_party_integration/krino/KrinoLevelSetPolicy.hpp"
#include "plato/third_party_integration/krino/test_utilities/KrinoTestFixture.hpp"
#include "plato/utilities/ContainerHelpers.hpp"
#include "plato/utilities/DataFilePath.hpp"

namespace plato::third_party_integration::krino::unittest
{
namespace
{
class KrinoLevelSetPolicyFixture : public test_utilities::KrinoTestFixture
{
};

const auto kFourTriTwoBlockMeshFilePath = utilities::data_file_path("four_tri_two_block.cdf");
constexpr auto kDecompositionMethod = std::string_view{"rib"};
}  // namespace

TEST_F(KrinoLevelSetPolicyFixture, SetupLevelSets)
{
    ASSERT_TRUE(kFourTriTwoBlockMeshFilePath.has_value());

    const auto tCheckFieldsAndBlocks = [](const std::set<std::string>& aExcludedBlocks,
                                          const std::vector<std::string>& aExpectedBlockNames,
                                          const plato::test_utilities::TestContext& aTestContext)
    {
        auto tMeshFromFile =
            std::make_unique<::krino::MeshFromFile>(kFourTriTwoBlockMeshFilePath.value().string(),
                                                    stk::EnvData::parallel_comm(), std::string{kDecompositionMethod});
        setup_level_sets(*tMeshFromFile, aExcludedBlocks);

        const auto& tFields = tMeshFromFile->meta_data().get_fields(stk::topology::rank_t::NODE_RANK);

        // Check fields
        ASSERT_EQ(tFields.size(), std::size_t{2}) << aTestContext;
        EXPECT_EQ(tFields.front()->name(), "coordinates") << aTestContext;
        EXPECT_EQ(tFields.back()->name(), "LS") << aTestContext;

        // Check blocks
        const auto& tParts = tMeshFromFile->meta_data().get_mesh_parts();
        auto tBlockNames = utilities::reserved_container<std::vector<std::string>>(tParts.size());
        std::ranges::transform(tParts, std::back_inserter(tBlockNames),
                               [](const auto* aPart) { return aPart->name(); });

        for (const auto& tExpectedBlock : aExpectedBlockNames)
        {
            EXPECT_NE(std::ranges::find(tBlockNames, tExpectedBlock), tBlockNames.end())
                << aTestContext << "Block: " << tExpectedBlock;
        }
        for (const auto& tExcludedBlock : aExcludedBlocks)
        {
            EXPECT_EQ(std::ranges::find(tBlockNames, tExcludedBlock + "_void"), tBlockNames.end())
                << aTestContext << "Excluded block: " << tExcludedBlock;
        }
    };

    tCheckFieldsAndBlocks({}, {"block_1_void", "block_2_void"}, TEST_CONTEXT("All blocks"));
    tCheckFieldsAndBlocks({"block_1"}, {"block_2_void"}, TEST_CONTEXT("Block 1 excluded"));
    tCheckFieldsAndBlocks({"block_2"}, {"block_1_void"}, TEST_CONTEXT("Block 2 excluded"));
}

TEST_F(KrinoLevelSetPolicyFixture, BlockParts)
{
    ASSERT_TRUE(kFourTriTwoBlockMeshFilePath.has_value());
    auto tMeshFromFile =
        std::make_unique<::krino::MeshFromFile>(kFourTriTwoBlockMeshFilePath.value().string(),
                                                stk::EnvData::parallel_comm(), std::string{kDecompositionMethod});

    const auto tToBlockNames = [](const stk::mesh::PartVector& aBlockParts)
    {
        auto tBlockNames = utilities::reserved_container<std::vector<std::string>>(aBlockParts.size());
        std::ranges::transform(aBlockParts, std::back_inserter(tBlockNames),
                               [](const auto& aBlockPart) { return aBlockPart->name(); });
        return tBlockNames;
    };
    // All block parts
    {
        const auto tAllBlocks = all_block_parts(*tMeshFromFile);
        const auto tExpectedBlockNames = std::vector<std::string>{"block_1", "block_2"};
        EXPECT_EQ(tToBlockNames(tAllBlocks), tExpectedBlockNames);
    }
    // Subset of block parts
    {
        const auto tBlocks = all_blocks_except(*tMeshFromFile, {"block_1"});
        const auto tExpectedBlockNames = std::vector<std::string>{"block_2"};
        EXPECT_EQ(tToBlockNames(tBlocks), tExpectedBlockNames);
    }
    {
        const auto tBlocks = all_blocks_except(*tMeshFromFile, {"block_2"});
        const auto tExpectedBlockNames = std::vector<std::string>{"block_1"};
        EXPECT_EQ(tToBlockNames(tBlocks), tExpectedBlockNames);
    }
}

}  // namespace plato::third_party_integration::krino::unittest
