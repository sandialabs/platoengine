#include <gtest/gtest.h>

#include <filesystem>
#include <string_view>

#include "plato/mesh/Mesh.hpp"
#include "plato/test_utilities/TestContext.hpp"
#include "plato/test_utilities/TestDataFilePath.hpp"
#include "plato/third_party_integration/stk_io/CommandGenerator.hpp"
#include "plato/third_party_integration/stk_io/Utilities.hpp"

namespace plato::mesh::unittest
{
namespace
{
constexpr auto kMeshName = std::string_view{"example.exo"};
constexpr auto kGenerator = third_party_integration::stk_io::CommandGenerator{{3, 1, 1}, {0, 0, 0}, {3, 1, 1}};
constexpr auto kExpectedNumberOfBlocks = 2u;

Mesh test_two_block_mesh(const test_utilities::TestContext& aTestContext)
{
    constexpr auto tMeshName = std::string_view{"box_2x4x10_hex_and_tet.cdf"};
    const auto tFilePath = test_utilities::test_data_file_path(tMeshName);
    EXPECT_TRUE(tFilePath) << aTestContext;
    return Mesh(tFilePath.value());
}

void clean_up(const test_utilities::TestContext& aTestContext)
{
    EXPECT_TRUE(std::filesystem::exists(kMeshName)) << aTestContext;
    EXPECT_TRUE(std::filesystem::remove(kMeshName)) << aTestContext;
}

}  // namespace

TEST(Mesh, AccessorFunctions)
{
    third_party_integration::stk_io::write_mesh(kMeshName, kGenerator);
    const Mesh tMesh(kMeshName);
    const unsigned int tSpatialDimensions = 3;

    EXPECT_EQ(tMesh.numberOfElements(), kGenerator.numberOfElements());
    EXPECT_EQ(tMesh.numberOfNodes(), kGenerator.numberOfNodes());
    EXPECT_EQ(tMesh.spatialDimensions(), tSpatialDimensions);

    const auto tNodes = tMesh.flattenedNodalCoordinates();
    EXPECT_EQ(tNodes.size(), tMesh.numberOfNodes() * tMesh.spatialDimensions());

    clean_up(TEST_CONTEXT("Deleting files in AccessorFunctions"));
}

TEST(Mesh, Coordinates)
{
    third_party_integration::stk_io::write_mesh(kMeshName, kGenerator);
    const auto tMesh = Mesh{kMeshName};

    const auto tNodes = tMesh.nodalCoordinates();
    EXPECT_EQ(tNodes.size(), tMesh.numberOfNodes());

    const auto tCentroids = tMesh.elementCentroids();
    EXPECT_EQ(tCentroids.size(), tMesh.numberOfElements());

    clean_up(TEST_CONTEXT("Deleting files in Coordinates"));
}

TEST(Mesh, Volume)
{
    third_party_integration::stk_io::write_mesh(kMeshName, kGenerator);
    const auto tMesh = Mesh{kMeshName};

    EXPECT_DOUBLE_EQ(tMesh.volume(), kGenerator.volume());

    clean_up(TEST_CONTEXT("Deleting files in Coordinates"));
}

TEST(Mesh, NumberOfBlocks)
{
    const auto tMesh = test_two_block_mesh(TEST_CONTEXT("Number of blocks"));
    EXPECT_EQ(tMesh.numberOfBlocks(), kExpectedNumberOfBlocks);
}

TEST(Mesh, BlockIDsAndNames)
{
    const auto tMesh = test_two_block_mesh(TEST_CONTEXT("Block ids and names"));
    const auto tBlockIDsAndNames = tMesh.blockData();
    ASSERT_EQ(tBlockIDsAndNames.size(), kExpectedNumberOfBlocks);

    EXPECT_EQ(tBlockIDsAndNames.front().mID, 1);
    EXPECT_EQ(tBlockIDsAndNames.front().mName, "block_1");

    EXPECT_EQ(tBlockIDsAndNames.back().mID, 2);
    EXPECT_EQ(tBlockIDsAndNames.back().mName, "block_2");
}

}  // namespace plato::mesh::unittest
