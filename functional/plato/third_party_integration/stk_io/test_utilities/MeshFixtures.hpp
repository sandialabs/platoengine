#ifndef PLATO_THIRD_PARTY_INTEGRATION_STK_IO_TEST_UTILITIES_MESHFIXTURES
#define PLATO_THIRD_PARTY_INTEGRATION_STK_IO_TEST_UTILITIES_MESHFIXTURES

#include <gtest/gtest.h>

#include <filesystem>

#include "plato/third_party_integration/stk_io/CommandGenerator.hpp"

namespace plato::third_party_integration::stk_io::test_utilities
{
/// @brief A base class fixture that creates a mesh using CommandGenerator on construction, and deletes it on
/// destruction.
class MeshGeneratingTestFixture : public ::testing::Test
{
   protected:
    MeshGeneratingTestFixture(const std::filesystem::path& aMeshName,
                              const third_party_integration::stk_io::CommandGenerator& aCommandGenerator);
    ~MeshGeneratingTestFixture();

    std::filesystem::path mMeshFilePath;
    third_party_integration::stk_io::CommandGenerator mCommandGenerator;
};

/// @brief A mesh creating test fixture that creates a 3x1x1 hex mesh.
class OneBlock3x1x1HexMesh : public MeshGeneratingTestFixture
{
   protected:
    OneBlock3x1x1HexMesh();
};

/// @brief A mesh creating test fixture that creates a 3x1x1 hex mesh with two nodesets on the x plus and minus faces.
class OneBlock3x1x1HexMeshWithNodeSets : public MeshGeneratingTestFixture
{
   protected:
    OneBlock3x1x1HexMeshWithNodeSets();
};

/// @brief A mesh test fixture that provides the path to a specific two-block mesh and has members
/// giving its node and element sizes.
class TwoBlockMeshOnDisk : public ::testing::Test
{
   protected:
    TwoBlockMeshOnDisk();

    std::filesystem::path mMeshFilePath;

    constexpr static auto mMeshFileName = std::string_view{"box_2x4x10_hex_and_tet.cdf"};
    constexpr static auto mExpectedNumberOfBlocks = 2U;
    constexpr static auto mExpectedNumberOfElementsInBlock1 = 273U;
    constexpr static auto mExpectedNumberOfElementsInBlock2 = 40U;
    constexpr static auto mExpectedNumberOfNodesInBlock1 = 93U;
    constexpr static auto mExpectedNumberOfNodesInBlock2 = 90U;
    constexpr static auto mExpectedVolume = 80.0;
    constexpr static auto mSmallestElementVolume = 0.080348748056025029;
};

/// @brief A mesh test fixture that provides the path to a 2D, non-uniform hex mesh
/// giving its node and element sizes.
class TwoDNonUniformHexMesh : public ::testing::Test
{
   protected:
    TwoDNonUniformHexMesh();

    std::filesystem::path mMeshFilePath;

    constexpr static auto mMeshFileName = std::string_view{"rectangle_3x4_quad4.cdf"};
    constexpr static auto mExpectedNumberOfBlocks = 1U;
    constexpr static auto mExpectedNumberOfElements = 29U;
    constexpr static auto mExpectedNumberOfNodes = 41U;
    constexpr static auto mExpectedArea = 12.0;
    constexpr static auto mExpectedSmallestElementArea = 0.15715566967755179;
};

/// @brief A mesh test fixture providing a 2D mesh with 3 blocks. The nodes and elements
/// are ordered so that the first nodes/elements do not belong to the first block.
class TwoDThreeBlockMesh : public ::testing::Test
{
   protected:
    TwoDThreeBlockMesh();
    ~TwoDThreeBlockMesh();

    std::filesystem::path mMeshFilePath = "two_d_three_block.exo";

    constexpr static auto mMeshDescription = std::string_view{
        "textmesh:"
        "0,3,QUAD_4_2D,2,5,6,3,block_3\n"
        "0,1,TRI_3_2D,1,2,3,block_2\n"
        "0,2,TRI_3_2D,3,4,1,block_2\n"
        "0,4,TRI_3_2D,2,7,9,block_1\n"
        "0,5,TRI_3_2D,7,8,9,block_1\n"
        "0,6,TRI_3_2D,8,5,9,block_1\n"
        "0,7,TRI_3_2D,5,2,9,block_1\n"
        "|coordinates: -2,0,0,0,0,1,-2,1,2,0,2,1,0,-1,2,-1,1,-0.5"
        "|dimension:2"};

    constexpr static auto mExpectedNumberOfBlocks = 3U;
    constexpr static auto mExpectedNumberOfElements = 7U;
    constexpr static auto mExpectedNumberOfElementsInBlock1 = 4U;
    constexpr static auto mExpectedNumberOfElementsInBlock2 = 2U;
    constexpr static auto mExpectedNumberOfElementsInBlock3 = 1U;
    constexpr static auto mExpectedNumberOfNodes = 9U;
    constexpr static auto mExpectedNumberOfNodesInBlock1 = 5U;
    constexpr static auto mExpectedNumberOfNodesInBlock2 = 4U;
    constexpr static auto mExpectedNumberOfNodesInBlock3 = 4U;
    constexpr static auto mBlock1Ordinal = 20U;
    constexpr static auto mBlock2Ordinal = 21U;
    constexpr static auto mBlock3Ordinal = 22U;
};

/// @brief A mesh test fixture providing a 2D mesh with 2 blocks with non-standard block names.
class TwoDTwoBlockMesh : public ::testing::Test
{
   protected:
    TwoDTwoBlockMesh();
    ~TwoDTwoBlockMesh();

    std::filesystem::path mMeshFilePath = "two_d_two_block.exo";

    constexpr static auto mMeshDescription = std::string_view{
        "textmesh:"
        "0,3,QUAD_4_2D,1,2,5,6,fixed\n"
        "0,1,TRI_3_2D,2,3,4,design\n"
        "0,2,TRI_3_2D,2,4,5,design\n"
        "|coordinates: 0,0,0.5,0,2,0,2,1,0.5,1,0,1"
        "|dimension:2"};

    constexpr static auto mExpectedNumberOfBlocks = 2U;
    constexpr static auto mExpectedNumberOfElements = 3U;
    constexpr static auto mExpectedNumberOfElementsInBlock1 = 1U;
    constexpr static auto mExpectedNumberOfElementsInBlock2 = 2U;
    constexpr static auto mExpectedNumberOfNodes = 6U;
    constexpr static auto mExpectedNumberOfNodesInBlock1 = 4U;
    constexpr static auto mExpectedNumberOfNodesInBlock2 = 4U;
};

constexpr auto kTwoDTriMesh = std::string_view{
    "textmesh:"
    "0,1,TRI_3_2D,3,1,4,block_1\n"
    "0,2,TRI_3_2D,1,2,4,block_1\n"
    "0,3,TRI_3_2D,2,5,4,block_1\n"
    "0,4,TRI_3_2D,5,7,4,block_2\n"
    "0,5,TRI_3_2D,7,6,4,block_2\n"
    "0,6,TRI_3_2D,6,3,4,block_2\n"
    "|coordinates: 0,0,0.125,0,0,0.125,0.0625,0.125,0.125,0.125,0,0.25,0.125,0.25"
    "|dimension:2"};

}  // namespace plato::third_party_integration::stk_io::test_utilities

#endif
