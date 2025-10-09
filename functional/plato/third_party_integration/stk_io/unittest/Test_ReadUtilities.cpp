#include <gtest/gtest.h>

#include <stk_mesh/base/BulkData.hpp>
#include <stk_mesh/base/MetaData.hpp>
#include <unordered_map>

#include "plato/test_utilities/TestContext.hpp"
#include "plato/third_party_integration/common/test_utilities/CoordinateTestUtilities.hpp"
#include "plato/third_party_integration/stk_io/BlockUtilities.hpp"
#include "plato/third_party_integration/stk_io/ReadUtilities.hpp"
#include "plato/third_party_integration/stk_io/WriteUtilities.hpp"
#include "plato/third_party_integration/stk_io/test_utilities/MeshFixtures.hpp"
#include "plato/third_party_integration/stk_io/test_utilities/MeshWithFieldWriter.hpp"
#include "plato/utilities/DataFilePath.hpp"

namespace plato::third_party_integration::stk_io::unittest
{

using ElementDensityMesh = test_utilities::MeshWithElementDensities;
using NodalDensityMesh = test_utilities::MeshWithNodalDensities;
using third_party_integration::stk_io::test_utilities::TwoBlockMeshOnDisk;
using third_party_integration::stk_io::test_utilities::TwoDThreeBlockMesh;
using PartReferenceVector = std::vector<std::reference_wrapper<const stk::mesh::Part>>;

TEST(ReadUtilities, SpatialDimensions3)
{
    const CommandGenerator tCommandGenerator{{2, 2, 2}};
    const auto tMesh = generate_bulk_data(tCommandGenerator);
    ASSERT_TRUE(tMesh);
    EXPECT_EQ(spatial_dimensions(*tMesh), 3u);
}

TEST(ReadUtilities, SpatialDimensions2)
{
    const auto tMesh = read_mesh_bulk_data(plato::utilities::data_file_path("rectangle_3x4_tri3.cdf").value());
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
    {
        const auto tResult = read_element_field(mMeshName, mFieldName);
        ASSERT_FALSE(tResult.empty());
        EXPECT_EQ(tResult, mGoldNumbering);
    }
    {
        constexpr double tTimeStep = 1.0;
        const auto tResult = read_element_field(mMeshName, mFieldName, tTimeStep);
        ASSERT_FALSE(tResult.empty());
        EXPECT_EQ(tResult, mGoldNumbering);
    }
    {
        constexpr double tTimeStep = 19.25;
        const auto tResult = read_element_field(mMeshName, mFieldName, tTimeStep);
        EXPECT_TRUE(tResult.empty());
    }
}

TEST_F(NodalDensityMesh, ReadNodalField)
{
    {
        const auto tResult = read_nodal_field(mMeshName, mFieldName);
        ASSERT_FALSE(tResult.empty());
        EXPECT_EQ(tResult, mGoldNumbering);
    }
    {
        constexpr double tTimeStep = 1.0;
        const auto tResult = read_nodal_field(mMeshName, mFieldName, tTimeStep);
        ASSERT_FALSE(tResult.empty());
        EXPECT_EQ(tResult, mGoldNumbering);
    }
    {
        const auto tResult = read_nodal_field(mMeshName, mFieldName, LastTimeStep{});
        EXPECT_EQ(tResult, mGoldNumbering);
    }
    {
        constexpr double tTimeStep = 71.125;
        const auto tResult = read_nodal_field(mMeshName, mFieldName, tTimeStep);
        EXPECT_TRUE(tResult.empty());
    }
}

TEST_F(NodalDensityMesh, NodalFieldNames)
{
    const auto tResult = nodal_field_names(mMeshName);
    const std::vector<std::string> tGold{"coordinates", "Topology"};
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

TEST_F(NodalDensityMesh, TimeSteps)
{
    const auto tResult = time_steps(mMeshName);
    const auto tExpected = std::vector{1.0};
    EXPECT_EQ(tResult, tExpected);
}

TEST(ReadUtilities, GlobalNodeIDs)
{
    const auto tMeshPath = std::filesystem::path{"temp_mesh_save.exo"};
    constexpr auto tMesh = std::string_view{
        "textmesh:"
        "0,1,TET_4,15,11,12,13,block_1\n"
        "0,2,TET_4,16,15,12,13,block_1\n"
        "0,3,TET_4,16,17,15,13,block_1\n"
        "0,4,TET_4,16,14,17,13,block_1\n"
        "0,5,TET_4,16,12,14,13,block_1\n"
        "0,6,TET_4,16,18,17,14,block_1\n"
        "0,7,TET_4,19,15,16,17,block_2\n"
        "0,8,TET_4,20,19,16,17,block_2\n"
        "0,9,TET_4,20,21,19,17,block_2\n"
        "0,10,TET_4,20,18,21,17,block_2\n"
        "0,11,TET_4,20,16,18,17,block_2\n"
        "0,12,TET_4,20,22,21,18,block_2\n"
        "|coordinates: 0,-1,-1,0,0,-1,1,-1,-1,1,0,-1,0,-1,1,0,0,1,1,-1,1,1,0,1,0,-1,3,0,0,3,1,-1,3,1,0,3"
        "|dimension:3|sideset:name=my_ss;data=7,2,11,2"};  // data=<tet_id>,<side_id>,<tet_id>,<side_id>...
    stk_io::write_mesh(tMeshPath, tMesh);
    const auto tBulkData = stk_io::read_mesh_bulk_data(tMeshPath);
    {
        const std::vector<size_t> tPart1IDs = global_node_ids(
            *tBulkData, PartReferenceVector{std::cref(*tBulkData->mesh_meta_data().get_part("block_1"))});
        const auto tExpectedNodeIDs = std::vector<std::size_t>{11, 12, 13, 14, 15, 16, 17, 18};
        EXPECT_EQ(tPart1IDs, tExpectedNodeIDs);
    }
    {
        const std::vector<size_t> tPart1IDs = global_node_ids(
            *tBulkData, PartReferenceVector{std::cref(*tBulkData->mesh_meta_data().get_part("block_2"))});
        const auto tExpectedNodeIDs = std::vector<std::size_t>{15, 16, 17, 18, 19, 20, 21, 22};
        EXPECT_EQ(tPart1IDs, tExpectedNodeIDs);
    }
    std::filesystem::remove(tMeshPath);
}

}  // namespace plato::third_party_integration::stk_io::unittest
