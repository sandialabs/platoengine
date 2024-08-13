#include <Ioss_ElementBlock.h>
#include <Ioss_IOFactory.h>
#include <Ioss_NodeBlock.h>
#include <Ioss_Region.h>
#include <gtest/gtest.h>
#include <mpi.h>

#include <cmath>
#include <filesystem>
#include <numeric>
#include <stk_mesh/base/BulkData.hpp>
#include <stk_mesh/base/Comm.hpp>
#include <stk_mesh/base/MetaData.hpp>
#include <stk_util/parallel/Parallel.hpp>
#include <string_view>

#include "plato/test_utilities/TestDataFilePath.hpp"
#include "plato/third_party_integration/common/test_utilities/CoordinateTestUtilities.hpp"
#include "plato/third_party_integration/stk_io/CommandGenerator.hpp"
#include "plato/third_party_integration/stk_io/Utilities.hpp"
#include "plato/third_party_integration/stk_io/test_utilities/MeshFixtures.hpp"

namespace plato::third_party_integration::stk_io::unittest
{
namespace
{
using third_party_integration::stk_io::test_utilities::TwoBlockMeshOnDisk;

constexpr auto kTopologyFieldName = std::string_view{"topology"};

std::vector<double> read_nodal_density(const std::filesystem::path& aMeshName)
{
    Ioss::DatabaseIO* tResultsDb =
        Ioss::IOFactory::create("exodus", aMeshName.string(), Ioss::READ_MODEL, MPI_COMM_SELF);
    Ioss::Region tResults(tResultsDb);

    tResults.begin_state(1);
    Ioss::NodeBlock* tNb = tResults.get_node_blocks()[0];
    std::vector<double> tNodeFieldData;
    tNb->get_field_data(std::string{kTopologyFieldName}, tNodeFieldData);
    return tNodeFieldData;
}

std::vector<double> read_element_density(const std::filesystem::path& aMeshName)
{
    Ioss::DatabaseIO* tResultsDb =
        Ioss::IOFactory::create("exodus", aMeshName.string(), Ioss::READ_MODEL, MPI_COMM_SELF);
    Ioss::Region tResults(tResultsDb);

    tResults.begin_state(1);
    Ioss::ElementBlock* tEb = tResults.get_element_blocks()[0];
    std::vector<double> tElementFieldData;
    tEb->get_field_data(std::string{kTopologyFieldName}, tElementFieldData);
    return tElementFieldData;
}
}  // namespace

TEST(STKUtilities, CommandGeneratorWriteMeshToDisk)
{
    const CommandGenerator tCommandGenerator{{2, 2, 2}};
    const std::string_view tMeshFileName{"mesh.exo"};
    write_mesh(tMeshFileName, tCommandGenerator);
    EXPECT_TRUE(std::filesystem::exists(tMeshFileName));
    EXPECT_TRUE(std::filesystem::remove(tMeshFileName));
}

TEST(STKUtilities, NumberOfNodesAndElementsFromBulk)
{
    const CommandGenerator tCommandGenerator{{2, 2, 2}};
    const auto tMesh = generate_bulk_data(tCommandGenerator);
    ASSERT_TRUE(tMesh);
    EXPECT_EQ(node_size(*tMesh), tCommandGenerator.numberOfNodes());
    EXPECT_EQ(element_size(*tMesh), tCommandGenerator.numberOfElements());
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

TEST(STKUtilities, SpatialDimensions3)
{
    const CommandGenerator tCommandGenerator{{2, 2, 2}};
    const auto tMesh = generate_bulk_data(tCommandGenerator);
    ASSERT_TRUE(tMesh);
    EXPECT_EQ(spatial_dimensions(*tMesh), 3u);
}

TEST(STKUtilities, SpatialDimensions2)
{
    const auto tMesh =
        read_mesh_bulk_data(plato::test_utilities::test_data_file_path("rectangle_3x4_tri3.cdf").value());
    ASSERT_TRUE(tMesh);
    EXPECT_EQ(spatial_dimensions(*tMesh), 2u);
}

TEST(STKUtilities, ReadCoordinates)
{
    const CommandGenerator tCommandGenerator;
    const std::vector<double> gold = {0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 1, 0, 0, 0, 1, 1, 0, 1, 0, 1, 1, 1, 1, 1};
    const auto tMesh = generate_bulk_data(tCommandGenerator);
    ASSERT_TRUE(tMesh);
    const std::vector<double> res = flattened_nodal_coordinates(*tMesh);
    EXPECT_EQ(gold, res);
    EXPECT_EQ(node_size(*tMesh), tCommandGenerator.numberOfNodes());
}

TEST(STKUtilities, ReadCoordinatesCoordinate)
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

TEST(STKUtilities, WriteDensityField)
{
    constexpr std::string_view tInputFileName = "brick.exo";
    const auto tData = std::unordered_map<std::size_t, double>{{1, 1.0}, {2, 2.0}, {3, 3.0}, {4, 4.0},
                                                              {5, 5.0}, {6, 6.0}, {7, 7.0}, {8, 8.0}};

    auto tExpected = std::vector<double>(tData.size());
    std::iota(tExpected.begin(), tExpected.end(), 1.0);
    constexpr std::string_view tOutputFileName = "brick-out.exo";

    // Nodal
    {
        write_bulk_data(tInputFileName, generate_bulk_data(CommandGenerator{}));
        write_nodal_density(tInputFileName, tData, tOutputFileName);
        const auto tResult = read_nodal_density(tOutputFileName);
        EXPECT_EQ(tResult, tExpected);
    }
    // Element
    {
        write_bulk_data(tInputFileName, generate_bulk_data(CommandGenerator{{2, 2, 2}}));
        write_element_density(tInputFileName, tData, tOutputFileName);
        const auto tResult = read_element_density(tOutputFileName);
        EXPECT_EQ(tResult, tExpected);
    }

    EXPECT_TRUE(std::filesystem::remove(tInputFileName));
    EXPECT_TRUE(std::filesystem::remove(tOutputFileName));
}

}  // namespace plato::third_party_integration::stk_io::unittest
