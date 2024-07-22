#include <gtest/gtest.h>

#include <algorithm>
#include <numeric>

#include "plato/mesh/Mesh.hpp"
#include "plato/mesh/MeshBlocks.hpp"
#include "plato/mesh/unittest/Fixtures.hpp"

namespace plato::mesh::unittest
{
std::vector<std::size_t> sequential_vector(const std::size_t aStart, const std::size_t aSize)
{
    auto tIndices = std::vector<std::size_t>(aSize);
    std::iota(tIndices.begin(), tIndices.end(), aStart);
    return tIndices;
}

TEST_F(TwoBlockMeshOnDisk, BlockIDsAndNames)
{
    const auto tMesh = Mesh{mMeshFilePath};
    const auto tBlockIDsAndNames = MeshBlocks{tMesh}.blockData();
    ASSERT_EQ(tBlockIDsAndNames.size(), mExpectedNumberOfBlocks);

    EXPECT_EQ(tBlockIDsAndNames.front().mID, 1);
    EXPECT_EQ(tBlockIDsAndNames.front().mName, "block_1");

    EXPECT_EQ(tBlockIDsAndNames.back().mID, 2);
    EXPECT_EQ(tBlockIDsAndNames.back().mName, "block_2");

    // Check sorted post-condition
    EXPECT_TRUE(std::is_sorted(tBlockIDsAndNames.cbegin(), tBlockIDsAndNames.cend(),
                               [](const auto& tBlockDataLeft, const auto& tBlockDataRight)
                               { return tBlockDataLeft.mID < tBlockDataRight.mID; }));
}

TEST_F(OneBlock3x1x1HexMesh, BlockNodeIDs)
{
    const auto tMeshBase = Mesh{mMeshFilePath};
    const auto tMesh = MeshBlocks{tMeshBase};

    const auto tResultNodeIDs = tMesh.nodeIDs("block_1");
    EXPECT_EQ(tResultNodeIDs.size(), mCommandGenerator.numberOfNodes());
    constexpr auto tStartNodeID = 1u;
    const auto tExpectedNodeIDs = sequential_vector(tStartNodeID, mCommandGenerator.numberOfNodes());
    EXPECT_EQ(tResultNodeIDs, tExpectedNodeIDs);

    EXPECT_TRUE(tMesh.nodeIDs("an-iguana-is-not-a-block").empty());
}

TEST_F(TwoBlockMeshOnDisk, BlockNodeIDs)
{
    const auto tMeshBase = Mesh{mMeshFilePath};
    const auto tMesh = MeshBlocks{tMeshBase};

    const auto tResultNodeIDsBlock1 = tMesh.nodeIDs("block_1");
    EXPECT_EQ(tResultNodeIDsBlock1.size(), mExpectedNumberOfNodesInBlock1);
    constexpr auto tStartNodeIDBlock1 = 1u;
    const auto tExpectedNodeIDsBlock1 = sequential_vector(tStartNodeIDBlock1, mExpectedNumberOfNodesInBlock1);
    EXPECT_EQ(tExpectedNodeIDsBlock1, tResultNodeIDsBlock1);

    const auto tResultNodeIDsBlock2 = tMesh.nodeIDs("block_2");
    EXPECT_EQ(tResultNodeIDsBlock2.size(), mExpectedNumberOfNodesInBlock2);
    constexpr auto tStartNodeIDBlock2 = mExpectedNumberOfNodesInBlock1 + 1u;
    const auto tExpectedNodeIDsBlock2 = sequential_vector(tStartNodeIDBlock2, mExpectedNumberOfNodesInBlock2);
    EXPECT_EQ(tExpectedNodeIDsBlock2, tResultNodeIDsBlock2);
}

TEST_F(OneBlock3x1x1HexMesh, BlockElementIDs)
{
    const auto tMeshBase = Mesh{mMeshFilePath};
    const auto tMesh = MeshBlocks{tMeshBase};

    const auto tResultNodeIDs = tMesh.elementIDs("block_1");
    EXPECT_EQ(tResultNodeIDs.size(), mCommandGenerator.numberOfElements());
    constexpr auto tStartNodeID = 1u;
    const auto tExpectedNodeIDs = sequential_vector(tStartNodeID, mCommandGenerator.numberOfElements());
    EXPECT_EQ(tResultNodeIDs, tExpectedNodeIDs);

    EXPECT_TRUE(tMesh.elementIDs("a-komodo-dragon-is-not-a-block").empty());
}

TEST_F(TwoBlockMeshOnDisk, BlockElementIDs)
{
    const auto tMeshBase = Mesh{mMeshFilePath};
    const auto tMesh = MeshBlocks{tMeshBase};

    const auto tResultElementIDsBlock1 = tMesh.elementIDs("block_1");
    EXPECT_EQ(tResultElementIDsBlock1.size(), mExpectedNumberOfElementsInBlock1);
    constexpr auto tStartElementIDBlock1 = 1u;
    const auto tExpectedElementIDsBlock1 = sequential_vector(tStartElementIDBlock1, mExpectedNumberOfElementsInBlock1);
    EXPECT_EQ(tExpectedElementIDsBlock1, tResultElementIDsBlock1);

    const auto tResultElementIDsBlock2 = tMesh.elementIDs("block_2");
    EXPECT_EQ(tResultElementIDsBlock2.size(), mExpectedNumberOfElementsInBlock2);
    constexpr auto tStartElementIDBlock2 = mExpectedNumberOfElementsInBlock1 + 1u;
    const auto tExpectedElementIDsBlock2 = sequential_vector(tStartElementIDBlock2, mExpectedNumberOfElementsInBlock2);
    EXPECT_EQ(tExpectedElementIDsBlock2, tResultElementIDsBlock2);
}

}  // namespace plato::mesh::unittest
