#include <gtest/gtest.h>

#include <stk_mesh/base/BulkData.hpp>
#include <stk_mesh/base/MetaData.hpp>
#include <unordered_map>

#include "plato/test_utilities/TestContext.hpp"
#include "plato/test_utilities/TestDataFilePath.hpp"
#include "plato/third_party_integration/common/test_utilities/CoordinateTestUtilities.hpp"
#include "plato/third_party_integration/stk_io/BlockUtilities.hpp"
#include "plato/third_party_integration/stk_io/ReadUtilities.hpp"
#include "plato/third_party_integration/stk_io/WriteUtilities.hpp"
#include "plato/third_party_integration/stk_io/test_utilities/MeshFixtures.hpp"
#include "plato/third_party_integration/stk_io/test_utilities/MeshWithFieldWriter.hpp"

namespace plato::third_party_integration::stk_io::unittest
{

using ElementDensityMesh = test_utilities::MeshWithElementDensities;
using NodalDensityMesh = test_utilities::MeshWithNodalDensities;
using third_party_integration::stk_io::test_utilities::TwoBlockMeshOnDisk;
using third_party_integration::stk_io::test_utilities::TwoDThreeBlockMesh;

TEST(ReadUtilities, SpatialDimensions3)
{
    const CommandGenerator tCommandGenerator{{2, 2, 2}};
    const auto tMesh = generate_bulk_data(tCommandGenerator);
    ASSERT_TRUE(tMesh);
    EXPECT_EQ(spatial_dimensions(*tMesh), 3u);
}

TEST(ReadUtilities, SpatialDimensions2)
{
    const auto tMesh =
        read_mesh_bulk_data(plato::test_utilities::test_data_file_path("rectangle_3x4_tri3.cdf").value());
    ASSERT_TRUE(tMesh);
    EXPECT_EQ(spatial_dimensions(*tMesh), 2u);
}

TEST(ReadUtilities, ReadCoordinatesCoordinate)
{
    const CommandGenerator tCommandGenerator;
    const std::vector<common::Coordinate> tGold = {{0, 0, 0}, {1, 0, 0}, {0, 1, 0}, {1, 1, 0},
                                                   {0, 0, 1}, {1, 0, 1}, {0, 1, 1}, {1, 1, 1}};
    const auto tMesh = generate_bulk_data(tCommandGenerator);
    ASSERT_TRUE(tMesh);
    const std::vector<common::Coordinate> tResult = nodal_coordinates(*tMesh);
    ASSERT_EQ(tGold.size(), tResult.size());
    for (unsigned int tIndex = 0; tIndex < tGold.size(); ++tIndex)
    {
        common::test_utilities::test_double_equality_of_components(tResult[tIndex], tGold[tIndex],
                                                                   TEST_CONTEXT("Read nodal coordinates"));
    }

    EXPECT_EQ(node_size(*tMesh), tCommandGenerator.numberOfNodes());
}

TEST(ReadUtilities, EntityIDs)
{
    const auto tMeshPath = std::filesystem::path{"temp_mesh.exo"};
    write_mesh(tMeshPath, test_utilities::kTwoDTriMesh);
    const auto tBulkData = read_mesh_bulk_data(tMeshPath);
    {
        constexpr auto tBlock1Ordinal = 20u;
        const auto tBlock1 = part_with_block_meta_data_ordinal(*tBulkData, tBlock1Ordinal);
        ASSERT_TRUE(tBlock1);
        const auto tResultElementIDs = element_ids(*tBulkData, tBlock1->get());
        const auto tExpectedElementIDs = std::vector<std::size_t>{1, 2, 3};
        EXPECT_EQ(tResultElementIDs, tExpectedElementIDs);

        const auto tResultNodeIDs = node_ids(*tBulkData, tBlock1->get());
        const auto tExpectedNodeIDs = std::vector<std::size_t>{1, 2, 3, 4, 5};
        EXPECT_EQ(tResultNodeIDs, tExpectedNodeIDs);
    }
    {
        constexpr auto tBlock2Ordinal = 21u;
        const auto tBlock2 = part_with_block_meta_data_ordinal(*tBulkData, tBlock2Ordinal);
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

TEST_F(ElementDensityMesh, CheckForFieldExistence)
{
    EXPECT_FALSE(nodal_field_exists(mMeshName, mFieldName));
    EXPECT_TRUE(element_field_exists(mMeshName, mFieldName));
}

TEST_F(NodalDensityMesh, CheckForFieldExistence)
{
    EXPECT_TRUE(nodal_field_exists(mMeshName, mFieldName));
    EXPECT_FALSE(element_field_exists(mMeshName, mFieldName));
}

TEST_F(ElementDensityMesh, ReadElementField)
{
    const auto tResult = read_element_field(mMeshName, mFieldName);
    EXPECT_EQ(tResult, mGoldNumbering);
}

TEST_F(NodalDensityMesh, ReadNodalField)
{
    const auto tResult = read_nodal_field(mMeshName, mFieldName);
    EXPECT_EQ(tResult, mGoldNumbering);
}

TEST_F(NodalDensityMesh, NodalFieldNames)
{
    const auto tResult = nodal_field_names(mMeshName);
    const std::vector<std::string> tGold{"coordinates", "topology"};
    EXPECT_EQ(tResult, tGold);
}

TEST_F(TwoDThreeBlockMesh, NumberOfNodesAndElementsFromBulkAndParts)
{
    const auto tBulkData = read_mesh_bulk_data(mMeshFilePath);
    const auto& tParts = tBulkData->mesh_meta_data().get_mesh_parts();
    constexpr auto tExpectedNumberOfParts = 3u;
    ASSERT_EQ(tParts.size(), tExpectedNumberOfParts);

    {
        const auto tBlock3Parts = PartReferenceVector{std::cref(*tParts.back())};
        const auto tNodalIDs = node_ids(*tBulkData, tBlock3Parts);
        EXPECT_EQ(tNodalIDs.size(), 4U);
        const std::vector<std::size_t> tGold{2U, 3U, 5U, 6U};
        EXPECT_EQ(tGold, tNodalIDs);
    }
    {
        const auto tAllParts = PartReferenceVector{std::cref(*tParts[0]), std::cref(*tParts[1]), std::cref(*tParts[2])};
        const auto tNodalIDs = node_ids(*tBulkData, tAllParts);
        EXPECT_EQ(tNodalIDs.size(), 9U);
        const std::vector<std::size_t> tGold{1U, 2U, 3U, 4U, 5U, 6U, 7U, 8U, 9U};
        EXPECT_EQ(tGold, tNodalIDs);
    }
}

TEST_F(TwoBlockMeshOnDisk, NumberOfNodesAndElementsFromBulkAndParts)
{
    const auto tBulkData = read_mesh_bulk_data(mMeshFilePath);
    const auto& tParts = tBulkData->mesh_meta_data().get_mesh_parts();
    constexpr auto tExpectedNumberOfParts = 2u;
    ASSERT_EQ(tParts.size(), tExpectedNumberOfParts);

    const auto tCheckCounts = [&tBulkData](const unsigned int aExpectedNodeSize,
                                           const unsigned int aExpectedElementSize, const PartReferenceVector& aParts,
                                           const plato::test_utilities::TestContext& aTestContext)
    {
        EXPECT_EQ(aExpectedNodeSize, node_size(*tBulkData, aParts)) << aTestContext;
        EXPECT_EQ(aExpectedElementSize, element_size(*tBulkData, aParts)) << aTestContext;
    };

    // Test full mesh result is same as parts list
    const auto tAllParts = PartReferenceVector{std::cref(*tParts.front()), std::cref(*tParts.back())};
    tCheckCounts(node_size(*tBulkData), element_size(*tBulkData), tAllParts, TEST_CONTEXT("Full mesh vs. all blocks"));
    // Block 1
    const auto tBlock1Parts = PartReferenceVector{std::cref(*tParts.front())};
    tCheckCounts(mExpectedNumberOfNodesInBlock1, mExpectedNumberOfElementsInBlock1, tBlock1Parts,
                 TEST_CONTEXT("Block 1"));
    // Block 2
    const auto tBlock2Parts = PartReferenceVector{std::cref(*tParts.back())};
    tCheckCounts(mExpectedNumberOfNodesInBlock2, mExpectedNumberOfElementsInBlock2, tBlock2Parts,
                 TEST_CONTEXT("Block 2"));
}

}  // namespace plato::third_party_integration::stk_io::unittest
