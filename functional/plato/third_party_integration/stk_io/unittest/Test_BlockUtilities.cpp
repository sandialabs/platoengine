#include <gtest/gtest.h>

#include <stk_mesh/base/BulkData.hpp>

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

    EXPECT_EQ(tBlockIDsAndNames.front().mID, 1u);
    EXPECT_EQ(tBlockIDsAndNames.front().mName, "block_1");
    EXPECT_EQ(tBlockIDsAndNames.back().mID, 2u);
    EXPECT_EQ(tBlockIDsAndNames.back().mName, "block_2");

    // Check sorted post-condition
    EXPECT_TRUE(std::is_sorted(tBlockIDsAndNames.cbegin(), tBlockIDsAndNames.cend(),
                               [](const auto& tBlockDataLeft, const auto& tBlockDataRight)
                               { return tBlockDataLeft.mID < tBlockDataRight.mID; }));
}

TEST(BlockUtilities, PartWithBlockName)
{
    const auto tTwoBlockMesh = test_mesh(TEST_CONTEXT("Bulk data with block name"));
    {
        const auto tBlock1 = part_with_block_name(*tTwoBlockMesh, "block_1");
        ASSERT_TRUE(tBlock1);
        EXPECT_EQ(element_size(*tTwoBlockMesh, tBlock1->get()), kExpectedNumberOfElementsInBlock1);
        EXPECT_EQ(node_size(*tTwoBlockMesh, tBlock1->get()), kExpectedNumberOfNodesInBlock1);
    }
    {
        const auto tBlock2 = part_with_block_name(*tTwoBlockMesh, "block_2");
        ASSERT_TRUE(tBlock2);
        EXPECT_EQ(element_size(*tTwoBlockMesh, tBlock2->get()), kExpectedNumberOfElementsInBlock2);
        EXPECT_EQ(node_size(*tTwoBlockMesh, tBlock2->get()), kExpectedNumberOfNodesInBlock2);
    }
}

}  // namespace plato::third_party_integration::stk_io::unittest
