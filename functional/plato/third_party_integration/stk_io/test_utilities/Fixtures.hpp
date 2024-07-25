#ifndef PLATO_THIRD_PARTY_INTEGRATION_STK_IO_TEST_UTILITIES_FIXTURES
#define PLATO_THIRD_PARTY_INTEGRATION_STK_IO_TEST_UTILITIES_FIXTURES

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

/// @brief A mesh test fixture that provides the path to a specific two-block mesh and has members
/// giving its node and element sizes.
class TwoBlockMeshOnDisk : public ::testing::Test
{
   protected:
    TwoBlockMeshOnDisk();

    std::filesystem::path mMeshFilePath;

    constexpr static auto mMeshFileName = std::string_view{"box_2x4x10_hex_and_tet.cdf"};
    constexpr static auto mExpectedNumberOfBlocks = 2u;
    constexpr static auto mExpectedNumberOfElementsInBlock1 = 273u;
    constexpr static auto mExpectedNumberOfElementsInBlock2 = 40u;
    constexpr static auto mExpectedNumberOfNodesInBlock1 = 93u;
    constexpr static auto mExpectedNumberOfNodesInBlock2 = 90u;
    constexpr static auto mExpectedVolume = 80.0;
};

/// @brief A mesh test fixture that provides the path to a 2D, non-uniform hex mesh
/// giving its node and element sizes.
class TwoDNonUniformHexMesh : public ::testing::Test
{
   protected:
    TwoDNonUniformHexMesh();

    std::filesystem::path mMeshFilePath;

    constexpr static auto mMeshFileName = std::string_view{"rectangle_3x4_quad4.cdf"};
    constexpr static auto mExpectedNumberOfBlocks = 1u;
    constexpr static auto mExpectedNumberOfElements = 29u;
    constexpr static auto mExpectedNumberOfNodes = 41u;
    constexpr static auto mExpectedArea = 12.0;
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

    constexpr static auto mExpectedNumberOfBlocks = 3u;
    constexpr static auto mExpectedNumberOfElementsInBlock1 = 1u;
    constexpr static auto mExpectedNumberOfElementsInBlock2 = 2u;
    constexpr static auto mExpectedNumberOfElementsInBlock3 = 4u;
    constexpr static auto mExpectedNumberOfNodes = 9u;
    constexpr static auto mExpectedNumberOfNodesInBlock1 = 4u;
    constexpr static auto mExpectedNumberOfNodesInBlock2 = 4u;
    constexpr static auto mExpectedNumberOfNodesInBlock3 = 5u;
};

}  // namespace plato::third_party_integration::stk_io::test_utilities

#endif
