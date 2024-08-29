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

#include "plato/test_utilities/TestContext.hpp"
#include "plato/test_utilities/TestDataFilePath.hpp"
#include "plato/third_party_integration/common/test_utilities/CoordinateTestUtilities.hpp"
#include "plato/third_party_integration/stk_io/CommandGenerator.hpp"
#include "plato/third_party_integration/stk_io/Utilities.hpp"
#include "plato/third_party_integration/stk_io/test_utilities/MeshFixtures.hpp"
#include "plato/third_party_integration/stk_io/test_utilities/MeshIOHelpers.hpp"

namespace plato::third_party_integration::stk_io::unittest
{
namespace
{
using third_party_integration::stk_io::test_utilities::TwoBlockMeshOnDisk;

constexpr auto kTopologyFieldName = std::string_view{"topology"};

void check_write_density(const std::filesystem::path& aInputFileName,
                         const std::unordered_map<std::size_t, double>& aData,
                         const std::filesystem::path& aOutputFileName,
                         const std::vector<double>& aExpected,
                         const plato::test_utilities::TestContext& aTestContext)
{
    const auto tScalarField = ScalarField{aData, "Topology", 1.0};
    // Nodal
    {
        write_bulk_data(aInputFileName, generate_bulk_data(CommandGenerator{}));
        write_nodal_scalar_field(aInputFileName, tScalarField, aOutputFileName);
        const auto tResult = test_utilities::read_nodal_field(aOutputFileName, kTopologyFieldName);
        EXPECT_EQ(tResult, aExpected) << aTestContext;
    }
    // Element
    {
        write_bulk_data(aInputFileName, generate_bulk_data(CommandGenerator{{2, 2, 2}}));
        write_element_scalar_field(aInputFileName, tScalarField, aOutputFileName);
        const auto tResult = test_utilities::read_element_field(aOutputFileName, kTopologyFieldName);
        EXPECT_EQ(tResult, aExpected) << aTestContext;
    }
    EXPECT_TRUE(std::filesystem::remove(aInputFileName)) << aTestContext;
    EXPECT_TRUE(std::filesystem::remove(aOutputFileName)) << aTestContext;
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

TEST(STKUtilities, WriteDensityFieldAllValuesExist)
{
    constexpr std::string_view tInputFileName = "brick.exo";
    const auto tData = std::unordered_map<std::size_t, double>{{1, 1.0}, {2, 2.0}, {3, 3.0}, {4, 4.0},
                                                               {5, 5.0}, {6, 6.0}, {7, 7.0}, {8, 8.0}};

    auto tExpected = std::vector<double>(tData.size());
    std::iota(tExpected.begin(), tExpected.end(), 1.0);
    constexpr std::string_view tOutputFileName = "brick-out.exo";

    check_write_density(tInputFileName, tData, tOutputFileName, tExpected, TEST_CONTEXT("All density values exist"));
}

TEST(STKUtilities, WriteDensityFieldSomeMissing)
{
    constexpr std::string_view tInputFileName = "brick.exo";
    const auto tData =
        std::unordered_map<std::size_t, double>{{1, 1.0}, {3, 3.0}, {5, 5.0}, {6, 6.0}, {7, 7.0}, {8, 8.0}};
    const auto tMissingGlobalIDs = std::vector<std::size_t>{2U, 4U};

    auto tExpected = std::vector<double>(tData.size() + tMissingGlobalIDs.size());
    std::iota(tExpected.begin(), tExpected.end(), 1.0);
    for (const auto tMissingID : tMissingGlobalIDs)
    {
        tExpected.at(tMissingID - 1) = 1.0;
    }
    constexpr std::string_view tOutputFileName = "brick-out.exo";

    check_write_density(tInputFileName, tData, tOutputFileName, tExpected,
                        TEST_CONTEXT("Some missing density field values"));
}

}  // namespace plato::third_party_integration::stk_io::unittest
