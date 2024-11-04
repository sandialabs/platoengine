#include <gtest/gtest.h>

#include <stk_io/FillMesh.hpp>
#include <stk_io/StkMeshIoBroker.hpp>

#include "plato/test_utilities/TestContext.hpp"
#include "plato/third_party_integration/stk_io/BlockUtilities.hpp"
#include "plato/third_party_integration/stk_io/ReadUtilities.hpp"
#include "plato/third_party_integration/stk_io/WriteUtilities.hpp"
#include "plato/third_party_integration/stk_io/test_utilities/MeshFixtures.hpp"
#include "plato/utilities/DataFilePath.hpp"

namespace plato::third_party_integration::stk_io::unittest
{
namespace
{
using third_party_integration::stk_io::test_utilities::OneBlock3x1x1HexMeshWithNodeSets;
using third_party_integration::stk_io::test_utilities::TwoBlockMeshOnDisk;
using third_party_integration::stk_io::test_utilities::TwoDThreeBlockMesh;

constexpr auto kExpectedNumberOfElementsInBlock1 = 273u;
constexpr auto kExpectedNumberOfElementsInBlock2 = 40u;
constexpr auto kExpectedNumberOfNodesInBlock1 = 93u;
constexpr auto kExpectedNumberOfNodesInBlock2 = 90u;

}  // namespace

TEST_F(TwoBlockMeshOnDisk, NumberOfBlocks)
{
    const auto tBulkData = read_mesh_bulk_data(mMeshFilePath);
    EXPECT_EQ(mExpectedNumberOfBlocks, block_size(*tBulkData));
}

TEST_F(TwoBlockMeshOnDisk, BlockData)
{
    const auto tBulkData = read_mesh_bulk_data(mMeshFilePath);

    const auto tBlockIDsAndNames = block_data(*tBulkData);

    ASSERT_EQ(tBlockIDsAndNames.size(), mExpectedNumberOfBlocks);

    EXPECT_EQ(tBlockIDsAndNames.front().mID, 1);
    EXPECT_EQ(tBlockIDsAndNames.front().mMetaDataOrdinal, 40u);
    EXPECT_EQ(tBlockIDsAndNames.front().mName, "block_1");
    EXPECT_EQ(tBlockIDsAndNames.back().mID, 2);
    EXPECT_EQ(tBlockIDsAndNames.back().mMetaDataOrdinal, 41u);
    EXPECT_EQ(tBlockIDsAndNames.back().mName, "block_2");

    // Check sorted post-condition
    EXPECT_TRUE(std::is_sorted(tBlockIDsAndNames.cbegin(), tBlockIDsAndNames.cend(),
                               [](const auto& tBlockDataLeft, const auto& tBlockDataRight)
                               { return tBlockDataLeft.mID < tBlockDataRight.mID; }));
}

TEST_F(TwoBlockMeshOnDisk, PartWithBlockNameAndID)
{
    const auto tTwoBlockMesh = read_mesh_bulk_data(mMeshFilePath);
    const auto tBlockData = block_data(*tTwoBlockMesh);
    {
        const auto tBlock = part_with_block_meta_data_ordinal(*tTwoBlockMesh, 40u);
        ASSERT_TRUE(tBlock);
        EXPECT_EQ(element_size(*tTwoBlockMesh, tBlock->get()), kExpectedNumberOfElementsInBlock1);
        EXPECT_EQ(node_size(*tTwoBlockMesh, tBlock->get()), kExpectedNumberOfNodesInBlock1);
    }
    {
        const auto tBlock = part_with_block_meta_data_ordinal(*tTwoBlockMesh, 41u);
        ASSERT_TRUE(tBlock);
        EXPECT_EQ(element_size(*tTwoBlockMesh, tBlock->get()), kExpectedNumberOfElementsInBlock2);
        EXPECT_EQ(node_size(*tTwoBlockMesh, tBlock->get()), kExpectedNumberOfNodesInBlock2);
    }
    {
        constexpr auto tInvalidBlockID = 420000;
        const auto tFalseBlock = part_with_block_meta_data_ordinal(*tTwoBlockMesh, tInvalidBlockID);
        EXPECT_FALSE(tFalseBlock);
    }
}

TEST(BlockUtilities, NodeIDsOneBlockNonSequential)
{
    const auto tMeshPath = std::filesystem::path{"temp_mesh.exo"};
    constexpr auto tMesh = std::string_view{"textmesh:0,1,HEX_8,1,2,3,4,11,12,13,14"};
    write_mesh(tMeshPath, tMesh);

    const auto tBulkData = read_mesh_bulk_data(tMeshPath);
    const auto& tParts = tBulkData->mesh_meta_data().get_mesh_parts();
    constexpr auto tExpectedNumberOfParts = 1U;
    ASSERT_EQ(tParts.size(), tExpectedNumberOfParts);

    const auto tResultIDs = node_ids(*tBulkData, *tParts.front());
    const auto tExpectedIDs = std::vector<std::size_t>{1, 2, 3, 4, 11, 12, 13, 14};

    EXPECT_EQ(tResultIDs, tExpectedIDs);

    std::filesystem::remove(tMeshPath);
}

TEST(BlockUtilities, NodalCoordinatesInBlock)
{
    const auto tMeshPath = std::filesystem::path{"temp_mesh.exo"};
    write_mesh(tMeshPath, test_utilities::kTwoDTriMesh);

    const auto tBulkData = read_mesh_bulk_data(tMeshPath);
    const auto& tParts = tBulkData->mesh_meta_data().get_mesh_parts();
    constexpr auto tExpectedNumberOfParts = 2U;
    ASSERT_EQ(tParts.size(), tExpectedNumberOfParts);

    const auto tAllNodalCoordinates = nodal_coordinates(*tBulkData);

    // All coordinates
    {
        const auto tCoordinateParts = std::vector{std::cref(*tParts.front()), std::cref(*tParts.back())};
        const auto tAllNodalCoordinatesFromParts = nodal_coordinates(*tBulkData, tCoordinateParts);
        EXPECT_EQ(tAllNodalCoordinatesFromParts, tAllNodalCoordinates);
    }
    // Block 1
    {
        const auto tExpectedCoordinates = std::vector<common::Coordinate>{
            {0, 0, 0}, {0.125, 0, 0}, {0, 0.125, 0}, {0.0625, 0.125, 0}, {0.125, 0.125, 0}};
        const auto tCoordinateParts = std::vector{std::cref(*tParts.front())};
        const auto tBlock1NodalCoordinatesFromParts = nodal_coordinates(*tBulkData, tCoordinateParts);
        EXPECT_EQ(tBlock1NodalCoordinatesFromParts, tExpectedCoordinates);
    }
    // Block 2
    {
        const auto tExpectedCoordinates = std::vector<common::Coordinate>{
            {0, 0.125, 0}, {0.0625, 0.125, 0}, {0.125, 0.125, 0}, {0, 0.25, 0}, {0.125, 0.25, 0}};
        const auto tCoordinateParts = std::vector{std::cref(*tParts.back())};
        const auto tBlock1NodalCoordinatesFromParts = nodal_coordinates(*tBulkData, tCoordinateParts);
        EXPECT_EQ(tBlock1NodalCoordinatesFromParts, tExpectedCoordinates);
    }

    std::filesystem::remove(tMeshPath);
}

TEST_F(OneBlock3x1x1HexMeshWithNodeSets, ElementBlockParts)
{
    const auto tBulkData = read_mesh_bulk_data(mMeshFilePath);
    const auto tElementBlocks = element_blocks_parts(*tBulkData);
    constexpr auto tNumberOfExpectedBlocks = 1U;
    EXPECT_EQ(tElementBlocks.size(), tNumberOfExpectedBlocks);
    EXPECT_EQ(element_size(*tBulkData, *tElementBlocks.front()), mCommandGenerator.numberOfElements());
}

TEST_F(TwoDThreeBlockMesh, ElementBlockParts)
{
    const auto tBulkData = read_mesh_bulk_data(mMeshFilePath);
    const auto tElementBlocks = element_blocks_parts(*tBulkData);
    constexpr auto tNumberOfExpectedBlocks = 3U;
    ASSERT_EQ(tElementBlocks.size(), tNumberOfExpectedBlocks);
    EXPECT_EQ(element_size(*tBulkData, *tElementBlocks.at(0)), mExpectedNumberOfElementsInBlock1);
    EXPECT_EQ(element_size(*tBulkData, *tElementBlocks.at(1)), mExpectedNumberOfElementsInBlock2);
    EXPECT_EQ(element_size(*tBulkData, *tElementBlocks.at(2)), mExpectedNumberOfElementsInBlock3);
}

TEST_F(OneBlock3x1x1HexMeshWithNodeSets, BlockSize)
{
    const auto tBulkData = read_mesh_bulk_data(mMeshFilePath);
    constexpr auto tNumberOfExpectedBlocks = 1U;
    EXPECT_EQ(block_size(*tBulkData), tNumberOfExpectedBlocks);
}

TEST_F(OneBlock3x1x1HexMeshWithNodeSets, BlockData)
{
    const auto tBulkData = read_mesh_bulk_data(mMeshFilePath);
    constexpr auto tNumberOfExpectedBlocks = 1U;
    const auto tBlockData = block_data(*tBulkData);
    ASSERT_EQ(tBlockData.size(), tNumberOfExpectedBlocks);

    EXPECT_EQ(tBlockData.front().mID, 1);
    EXPECT_EQ(tBlockData.front().mMetaDataOrdinal, 40U);
    EXPECT_EQ(tBlockData.front().mName, "block_1");
}

}  // namespace plato::third_party_integration::stk_io::unittest
