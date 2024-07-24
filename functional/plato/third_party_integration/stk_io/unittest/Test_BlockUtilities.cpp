#include <gtest/gtest.h>

#include <stk_io/FillMesh.hpp>
#include <stk_io/StkMeshIoBroker.hpp>

#include "plato/test_utilities/TestContext.hpp"
#include "plato/test_utilities/TestDataFilePath.hpp"
#include "plato/third_party_integration/stk_io/BlockUtilities.hpp"
#include "plato/third_party_integration/stk_io/Utilities.hpp"

namespace plato::third_party_integration::stk_io::unittest
{
namespace
{
constexpr auto kExpectedNumberOfBlocks = 2u;
constexpr auto kExpectedNumberOfElementsInBlock1 = 273u;
constexpr auto kExpectedNumberOfElementsInBlock2 = 40u;
constexpr auto kExpectedNumberOfNodesInBlock1 = 93u;
constexpr auto kExpectedNumberOfNodesInBlock2 = 90u;
constexpr auto tTwoDTriMesh = std::string_view{
    "textmesh:"
    "0,1,TRI_3_2D,3,1,4,block_1\n"
    "0,2,TRI_3_2D,1,2,4,block_1\n"
    "0,3,TRI_3_2D,2,5,4,block_1\n"
    "0,4,TRI_3_2D,5,7,4,block_2\n"
    "0,5,TRI_3_2D,7,6,4,block_2\n"
    "0,6,TRI_3_2D,6,3,4,block_2\n"
    "|coordinates: 0,0,0.125,0,0,0.125,0.0625,0.125,0.125,0.125,0,0.25,0.125,0.25"
    "|dimension:2"};

auto test_mesh(const plato::test_utilities::TestContext& aTestContext) -> std::shared_ptr<stk::mesh::BulkData>
{
    constexpr auto tMeshName = std::string_view{"box_2x4x10_hex_and_tet.cdf"};
    const auto tFilePath = test_utilities::test_data_file_path(tMeshName);
    EXPECT_TRUE(tFilePath) << aTestContext;
    return read_mesh_bulk_data(tFilePath.value());
}

}  // namespace

TEST(BlockUtilities, NumberOfBlocks)
{
    const auto tBulkData = test_mesh(TEST_CONTEXT("Number of blocks"));
    EXPECT_EQ(kExpectedNumberOfBlocks, block_size(*tBulkData));
}

TEST(BlockUtilities, BlockData)
{
    const auto tBulkData = test_mesh(TEST_CONTEXT("Block id"));

    const auto tBlockIDsAndNames = block_data(*tBulkData);

    ASSERT_EQ(tBlockIDsAndNames.size(), kExpectedNumberOfBlocks);

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

TEST(BlockUtilities, PartWithBlockNameAndID)
{
    const auto tTwoBlockMesh = test_mesh(TEST_CONTEXT("Bulk data with block name"));
    const auto tBlockData = block_data(*tTwoBlockMesh);
    for (const auto& [id, ordinal, name] : tBlockData)
    {
        std::cout << "block name: " << name << ", ordinal: " << ordinal << ", id = " << id << std::endl;
    }
    {
        const auto tBlockFromName = part_with_block_name(*tTwoBlockMesh, "block_1");
        const auto tBlockFromID = part_with_block_meta_data_ordinal(*tTwoBlockMesh, 40u);
        for (const auto& tBlock : {tBlockFromName, tBlockFromID})
        {
            ASSERT_TRUE(tBlock);
            EXPECT_EQ(element_size(*tTwoBlockMesh, tBlock->get()), kExpectedNumberOfElementsInBlock1);
            EXPECT_EQ(node_size(*tTwoBlockMesh, tBlock->get()), kExpectedNumberOfNodesInBlock1);
        }
    }
    {
        const auto tBlockFromName = part_with_block_name(*tTwoBlockMesh, "block_2");
        const auto tBlockFromID = part_with_block_meta_data_ordinal(*tTwoBlockMesh, 41u);
        for (const auto& tBlock : {tBlockFromName, tBlockFromID})
        {
            ASSERT_TRUE(tBlock);
            EXPECT_EQ(element_size(*tTwoBlockMesh, tBlock->get()), kExpectedNumberOfElementsInBlock2);
            EXPECT_EQ(node_size(*tTwoBlockMesh, tBlock->get()), kExpectedNumberOfNodesInBlock2);
        }
    }
    {
        auto tFalseBlock = part_with_block_name(*tTwoBlockMesh, "definitely-not-a-real-block-banana");
        EXPECT_FALSE(tFalseBlock);

        constexpr auto tInvalidBlockID = 420000;
        tFalseBlock = part_with_block_meta_data_ordinal(*tTwoBlockMesh, tInvalidBlockID);
        EXPECT_FALSE(tFalseBlock);
    }
}

TEST(BlockUtilities, EntityIDs)
{
    const auto tMeshPath = std::filesystem::path{"temp_mesh.exo"};
    write_mesh(tMeshPath, tTwoDTriMesh);
    const auto tBulkData = read_mesh_bulk_data(tMeshPath);
    {
        const auto tBlock1 = part_with_block_name(*tBulkData, "block_1");
        ASSERT_TRUE(tBlock1);
        const auto tResultElementIDs = element_ids(*tBulkData, tBlock1->get());
        const auto tExpectedElementIDs = std::vector<std::size_t>{1, 2, 3};
        EXPECT_EQ(tResultElementIDs, tExpectedElementIDs);

        const auto tResultNodeIDs = node_ids(*tBulkData, tBlock1->get());
        const auto tExpectedNodeIDs = std::vector<std::size_t>{1, 2, 3, 4, 5};
        EXPECT_EQ(tResultNodeIDs, tExpectedNodeIDs);
    }
    {
        const auto tBlock2 = part_with_block_name(*tBulkData, "block_2");
        ASSERT_TRUE(tBlock2);
        const auto tResultElementIDs = element_ids(*tBulkData, tBlock2->get());
        const auto tExpectedElementIDs = std::vector<std::size_t>{4, 5, 6};
        EXPECT_EQ(tResultElementIDs, tExpectedElementIDs);

        const auto tResultNodeIDs = node_ids(*tBulkData, tBlock2->get());
        const auto tExpectedNodeIDs = std::vector<std::size_t>{3, 4, 5, 6, 7};
        EXPECT_EQ(tResultNodeIDs, tExpectedNodeIDs);
    }
    std::filesystem::remove(tMeshPath);
}

TEST(BlockUtilities, NodeIDsOneBlockNonSequential)
{
    const auto tMeshPath = std::filesystem::path{"temp_mesh.exo"};
    constexpr auto tMesh = std::string_view{"textmesh:0,1,HEX_8,1,2,3,4,11,12,13,14"};
    write_mesh(tMeshPath, tMesh);

    const auto tBulkData = read_mesh_bulk_data(tMeshPath);
    const auto& tParts = tBulkData->mesh_meta_data().get_mesh_parts();
    constexpr auto tExpectedNumberOfParts = 1u;
    ASSERT_EQ(tParts.size(), tExpectedNumberOfParts);

    const auto tResultIDs = node_ids(*tBulkData, *tParts.front());
    const auto tExpectedIDs = std::vector<std::size_t>{1, 2, 3, 4, 11, 12, 13, 14};

    EXPECT_EQ(tResultIDs, tExpectedIDs);

    std::filesystem::remove(tMeshPath);
}

TEST(BlockUtilities, NodalCoordinatesInBlock)
{
    const auto tMeshPath = std::filesystem::path{"temp_mesh.exo"};
    write_mesh(tMeshPath, tTwoDTriMesh);

    const auto tBulkData = read_mesh_bulk_data(tMeshPath);
    const auto& tParts = tBulkData->mesh_meta_data().get_mesh_parts();
    constexpr auto tExpectedNumberOfParts = 2u;
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

}  // namespace plato::third_party_integration::stk_io::unittest
