#include <gtest/gtest.h>

#include "plato/test_utilities/TestContext.hpp"
#include "plato/test_utilities/TestDataFilePath.hpp"
#include "plato/third_party_integration/stk_io/Utilities.hpp"

namespace plato::third_party_integration::stk_io::unittest
{
namespace
{
auto test_mesh(const plato::test_utilities::TestContext& aTestContext) -> std::shared_ptr<stk::mesh::BulkData>
{
    constexpr auto tMeshName = std::string_view{"box_2x4x10_hex_and_tet.cdf"};
    const auto tFilePath = test_utilities::test_data_file_path(tMeshName);
    EXPECT_TRUE(tFilePath) << aTestContext;
    return read_mesh_bulk_data(tFilePath.value());
}
}  // namespace

TEST(BlockElementIdentifiers, NumberOfBlocks)
{
    const auto tBulkData = test_mesh(TEST_CONTEXT("Number of blocks"));
    constexpr auto tExpectedNumberOfBlocks = 2u;
    EXPECT_EQ(tExpectedNumberOfBlocks, block_size(*tBulkData));
}

TEST(BlockElementIdentifiers, BlockID)
{
    const auto tBulkData = test_mesh(TEST_CONTEXT("Block id"));

    {
        const auto tBlockID = block_id(*tBulkData, "block_1");
        ASSERT_TRUE(tBlockID);
        EXPECT_EQ(tBlockID.value(), 1u);
    }
    {
        const auto tBlockID = block_id(*tBulkData, "block_2");
        ASSERT_TRUE(tBlockID);
        EXPECT_EQ(tBlockID.value(), 2u);
    }
}

}  // namespace plato::third_party_integration::stk_io::unittest
