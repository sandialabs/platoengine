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
    EXPECT_EQ(kExpectedNumberOfBlocks, block_size(*tBulkData));
}

TEST(BlockElementIdentifiers, BlockData)
{
    const auto tBulkData = test_mesh(TEST_CONTEXT("Block id"));

    const auto tBlockIDsAndNames = block_data(*tBulkData);

    ASSERT_EQ(tBlockIDsAndNames.size(), kExpectedNumberOfBlocks);

    EXPECT_EQ(tBlockIDsAndNames.front().mID, 1u);
    EXPECT_EQ(tBlockIDsAndNames.front().mName, "block_1");
    EXPECT_EQ(tBlockIDsAndNames.back().mID, 2u);
    EXPECT_EQ(tBlockIDsAndNames.back().mName, "block_2");
}

}  // namespace plato::third_party_integration::stk_io::unittest
