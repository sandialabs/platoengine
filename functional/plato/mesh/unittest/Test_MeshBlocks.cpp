#include <gtest/gtest.h>

#include <algorithm>
#include <numeric>

#include "plato/mesh/Mesh.hpp"
#include "plato/mesh/MeshBlocks.hpp"
#include "plato/test_utilities/TestContext.hpp"
#include "plato/third_party_integration/stk_io/test_utilities/MeshFixtures.hpp"
#include "plato/utilities/Exception.hpp"
#include "plato/utilities/IndexRange.hpp"

namespace plato::mesh::unittest
{
namespace
{
using third_party_integration::stk_io::test_utilities::OneBlock3x1x1HexMesh;
using third_party_integration::stk_io::test_utilities::TwoBlockMeshOnDisk;
using third_party_integration::stk_io::test_utilities::TwoDNonUniformHexMesh;
using third_party_integration::stk_io::test_utilities::TwoDThreeBlockMesh;
using third_party_integration::stk_io::test_utilities::TwoDTwoBlockMesh;

std::vector<std::size_t> sequential_vector(const std::size_t aStart, const std::size_t aSize)
{
    auto tIndices = std::vector<std::size_t>(aSize);
    std::iota(tIndices.begin(), tIndices.end(), aStart);
    return tIndices;
}
}  // namespace

TEST_F(TwoBlockMeshOnDisk, BlockOrdinals)
{
    const auto tMesh = MeshBlocks{Mesh{mMeshFilePath}};

    const auto tBlock1Ordinal = tMesh.blockOrdinal("block_1");
    ASSERT_TRUE(tBlock1Ordinal);
    constexpr auto tExpectedBlock1Ordinal = 45u;
    EXPECT_EQ(tBlock1Ordinal.value(), tExpectedBlock1Ordinal);

    const auto tBlock2Ordinal = tMesh.blockOrdinal("block_2");
    ASSERT_TRUE(tBlock2Ordinal);
    constexpr auto tExpectedBlock2Ordinal = 46u;
    EXPECT_EQ(tBlock2Ordinal.value(), tExpectedBlock2Ordinal);

    // Non-exestent block
    const auto tBlock3Ordinal = tMesh.blockOrdinal("block_3");
    ASSERT_FALSE(tBlock3Ordinal);
}

TEST_F(TwoBlockMeshOnDisk, BlockIDsFromNames)
{
    const auto tMesh = MeshBlocks{Mesh{mMeshFilePath}};

    const auto tBlock1ID = tMesh.blockID("block_1");
    ASSERT_TRUE(tBlock1ID);
    constexpr auto tExpectedBlock1ID = 1;
    EXPECT_EQ(tBlock1ID.value(), tExpectedBlock1ID);

    const auto tBlock2ID = tMesh.blockID("block_2");
    ASSERT_TRUE(tBlock2ID);
    constexpr auto tExpectedBlock2ID = 2;
    EXPECT_EQ(tBlock2ID.value(), tExpectedBlock2ID);

    // Non-existent block
    const auto tBlock3ID = tMesh.blockID("block_3");
    ASSERT_FALSE(tBlock3ID);
}

TEST_F(TwoDThreeBlockMesh, BlockIDsFromOrdinals)
{
    const auto tMesh = MeshBlocks{Mesh{mMeshFilePath}};

    const auto tCheckIDFromOrdinal = [&tMesh](const std::string_view aBlockName,
                                              const MeshBlocks::BlockIDType aExpectedID,
                                              const plato::test_utilities::TestContext& aTestContext)
    {
        const auto tBlockOrdinal = tMesh.blockOrdinal(aBlockName);
        ASSERT_TRUE(tBlockOrdinal) << aTestContext;
        const auto tBlockID = tMesh.blockID(tBlockOrdinal.value());
        ASSERT_TRUE(tBlockID) << aTestContext;
        EXPECT_EQ(tBlockID.value(), aExpectedID) << aTestContext;
    };

    tCheckIDFromOrdinal(mBlockNames[0], 1, TEST_CONTEXT("Block 1"));
    tCheckIDFromOrdinal(mBlockNames[1], 2, TEST_CONTEXT("Block 2"));
    tCheckIDFromOrdinal(mBlockNames[2], 3, TEST_CONTEXT("Block 3"));
}

TEST_F(TwoDThreeBlockMesh, BlockIDs)
{
    const auto tMesh = Mesh{mMeshFilePath};
    const auto tBlockIDs = block_ids(tMesh, {mBlock1Ordinal, mBlock2Ordinal, mBlock3Ordinal});
    constexpr auto tExpectedSize = 3U;
    ASSERT_EQ(tBlockIDs.size(), tExpectedSize);
    EXPECT_EQ(tBlockIDs.at(0), 1U);
    EXPECT_EQ(tBlockIDs.at(1), 2U);
    EXPECT_EQ(tBlockIDs.at(2), 3U);
}

TEST_F(TwoDThreeBlockMesh, BlockData)
{
    const auto tMesh = Mesh{mMeshFilePath};
    const auto tBlockIDsAndNames = MeshBlocks{tMesh}.blockData();
    ASSERT_EQ(tBlockIDsAndNames.size(), mExpectedNumberOfBlocks);

    const auto tExpectedBlockIDs = std::vector{1, 2, 3};
    const auto tExpectedBlockOrdinals = std::vector{mBlock1Ordinal, mBlock2Ordinal, mBlock3Ordinal};
    const auto tExpectedBlockNames = std::vector<std::string>{mBlockNames[0], mBlockNames[1], mBlockNames[2]};

    for (const auto tIndex : utilities::IndexRange{mExpectedNumberOfBlocks})
    {
        EXPECT_EQ(tBlockIDsAndNames.at(tIndex).mID, tExpectedBlockIDs.at(tIndex));
        EXPECT_EQ(tBlockIDsAndNames.at(tIndex).mMetaDataOrdinal, tExpectedBlockOrdinals.at(tIndex));
        EXPECT_EQ(tBlockIDsAndNames.at(tIndex).mName, tExpectedBlockNames.at(tIndex));
    }

    // Check sorted post-condition
    EXPECT_TRUE(std::is_sorted(tBlockIDsAndNames.cbegin(), tBlockIDsAndNames.cend(),
                               [](const auto& tBlockDataLeft, const auto& tBlockDataRight)
                               { return tBlockDataLeft.mID < tBlockDataRight.mID; }));
}

TEST_F(OneBlock3x1x1HexMesh, BlockNodeIDs)
{
    const auto tMeshBase = Mesh{mMeshFilePath};
    const auto tMesh = MeshBlocks{tMeshBase};

    const auto tBlock1Ordinal = tMesh.blockOrdinal("block_1");
    ASSERT_TRUE(tBlock1Ordinal);
    const auto tResultNodeIDs = tMesh.nodeIDs(tBlock1Ordinal.value());
    EXPECT_EQ(tResultNodeIDs.size(), mCommandGenerator.numberOfNodes());
    constexpr auto tStartNodeID = 1u;
    const auto tExpectedNodeIDs = sequential_vector(tStartNodeID, mCommandGenerator.numberOfNodes());
    EXPECT_EQ(tResultNodeIDs, tExpectedNodeIDs);

    EXPECT_TRUE(tMesh.nodeIDs(tBlock1Ordinal.value() + 12345).empty());
}

TEST_F(TwoBlockMeshOnDisk, BlockNodeIDs)
{
    const auto tMeshBase = Mesh{mMeshFilePath};
    const auto tMesh = MeshBlocks{tMeshBase};

    const auto tBlock1Ordinal = tMesh.blockOrdinal("block_1");
    ASSERT_TRUE(tBlock1Ordinal);
    const auto tResultNodeIDsBlock1 = tMesh.nodeIDs(tBlock1Ordinal.value());
    EXPECT_EQ(tResultNodeIDsBlock1.size(), mExpectedNumberOfNodesInBlock1);
    constexpr auto tStartNodeIDBlock1 = 1u;
    const auto tExpectedNodeIDsBlock1 = sequential_vector(tStartNodeIDBlock1, mExpectedNumberOfNodesInBlock1);
    EXPECT_EQ(tExpectedNodeIDsBlock1, tResultNodeIDsBlock1);

    const auto tBlock2Ordinal = tMesh.blockOrdinal("block_2");
    ASSERT_TRUE(tBlock2Ordinal);
    const auto tResultNodeIDsBlock2 = tMesh.nodeIDs(tBlock2Ordinal.value());
    EXPECT_EQ(tResultNodeIDsBlock2.size(), mExpectedNumberOfNodesInBlock2);
    constexpr auto tStartNodeIDBlock2 = mExpectedNumberOfNodesInBlock1 + 1u;
    const auto tExpectedNodeIDsBlock2 = sequential_vector(tStartNodeIDBlock2, mExpectedNumberOfNodesInBlock2);
    EXPECT_EQ(tExpectedNodeIDsBlock2, tResultNodeIDsBlock2);
}

TEST_F(OneBlock3x1x1HexMesh, BlockElementIDs)
{
    const auto tMeshBase = Mesh{mMeshFilePath};
    const auto tMesh = MeshBlocks{tMeshBase};

    const auto tBlock1Ordinal = tMesh.blockOrdinal("block_1");
    ASSERT_TRUE(tBlock1Ordinal);
    const auto tResultNodeIDs = tMesh.elementIDs(tBlock1Ordinal.value());
    EXPECT_EQ(tResultNodeIDs.size(), mCommandGenerator.numberOfElements());
    constexpr auto tStartNodeID = 1u;
    const auto tExpectedNodeIDs = sequential_vector(tStartNodeID, mCommandGenerator.numberOfElements());
    EXPECT_EQ(tResultNodeIDs, tExpectedNodeIDs);

    EXPECT_TRUE(tMesh.elementIDs(tBlock1Ordinal.value() + 12345).empty());
}

TEST_F(TwoBlockMeshOnDisk, BlockElementIDs)
{
    const auto tMeshBase = Mesh{mMeshFilePath};
    const auto tMesh = MeshBlocks{tMeshBase};

    const auto tBlock1Ordinal = tMesh.blockOrdinal("block_1");
    ASSERT_TRUE(tBlock1Ordinal);
    const auto tResultElementIDsBlock1 = tMesh.elementIDs(tBlock1Ordinal.value());
    EXPECT_EQ(tResultElementIDsBlock1.size(), mExpectedNumberOfElementsInBlock1);
    constexpr auto tStartElementIDBlock1 = 1u;
    const auto tExpectedElementIDsBlock1 = sequential_vector(tStartElementIDBlock1, mExpectedNumberOfElementsInBlock1);
    EXPECT_EQ(tExpectedElementIDsBlock1, tResultElementIDsBlock1);

    const auto tBlock2Ordinal = tMesh.blockOrdinal("block_2");
    ASSERT_TRUE(tBlock2Ordinal);
    const auto tResultElementIDsBlock2 = tMesh.elementIDs(tBlock2Ordinal.value());
    EXPECT_EQ(tResultElementIDsBlock2.size(), mExpectedNumberOfElementsInBlock2);
    constexpr auto tStartElementIDBlock2 = mExpectedNumberOfElementsInBlock1 + 1u;
    const auto tExpectedElementIDsBlock2 = sequential_vector(tStartElementIDBlock2, mExpectedNumberOfElementsInBlock2);
    EXPECT_EQ(tExpectedElementIDsBlock2, tResultElementIDsBlock2);
}

TEST_F(TwoDNonUniformHexMesh, BlockNodeIDs)
{
    const auto tMeshBase = Mesh{mMeshFilePath};
    const auto tMesh = MeshBlocks{tMeshBase};

    const auto tBlock1Ordinal = tMesh.blockOrdinal("block_1");
    ASSERT_TRUE(tBlock1Ordinal);
    const auto tResultNodeIDsBlock1 = tMesh.nodeIDs(tBlock1Ordinal.value());
    // The node ids are from the node_num_map section in rectangle_3x4_quad.txt in test_utilities/data
    const auto tExpectedNodeIDs =
        std::vector<std::size_t>{5058, 5059, 5060, 5061, 5062, 5063, 5069, 5070, 5071, 5072, 5073, 5074, 5075, 5076,
                                 5077, 5086, 5087, 5088, 5089, 5095, 5096, 5097, 5102, 5103, 5104, 5105, 5106, 5107,
                                 5108, 5109, 5110, 5111, 5112, 5113, 5114, 5115, 5116, 5117, 5118, 5119, 5120};
    EXPECT_EQ(tResultNodeIDsBlock1, tResultNodeIDsBlock1);
}

TEST_F(TwoDNonUniformHexMesh, BlockElementIDs)
{
    const auto tMeshBase = Mesh{mMeshFilePath};
    const auto tMesh = MeshBlocks{tMeshBase};

    const auto tBlock1Ordinal = tMesh.blockOrdinal("block_1");
    ASSERT_TRUE(tBlock1Ordinal);
    const auto tResultElementIDsBlock1 = tMesh.elementIDs(tBlock1Ordinal.value());
    // The element ids are from the elem_num_map section in rectangle_3x4_quad.txt in test_utilities/data
    const auto tExpectedElementIDs =
        std::vector<std::size_t>{262, 263, 264, 265, 266, 267, 268, 269, 270, 271, 272, 273, 274, 275, 276,
                                 277, 278, 279, 280, 281, 282, 283, 284, 285, 286, 287, 288, 289, 290};
    EXPECT_EQ(tResultElementIDsBlock1, tResultElementIDsBlock1);
}

TEST_F(TwoDTwoBlockMesh, BlockNames)
{
    const auto tMesh = MeshBlocks{Mesh{mMeshFilePath}};

    const auto tExpectedNames = std::vector<std::string>{mBlockNames[0], mBlockNames[1]};
    EXPECT_EQ(tExpectedNames, tMesh.blockNames());
}

TEST_F(TwoDTwoBlockMesh, MeshConstructionWithNonExistingFixedBlockName)
{
    std::set<std::string> tFixedBlockNames{"non_existing_block_name"};
    EXPECT_THROW([[maybe_unused]] const auto tMesh = Mesh(mMeshFilePath, tFixedBlockNames),
                 plato::utilities::Exception);
}

}  // namespace plato::mesh::unittest
