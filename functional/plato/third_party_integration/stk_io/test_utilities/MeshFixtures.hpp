#ifndef PLATO_THIRD_PARTY_INTEGRATION_STK_IO_TEST_UTILITIES_MESHFIXTURES
#define PLATO_THIRD_PARTY_INTEGRATION_STK_IO_TEST_UTILITIES_MESHFIXTURES

#include <gtest/gtest.h>

#include <array>
#include <filesystem>
#include <string_view>
#include <unordered_map>

#include "plato/third_party_integration/stk_io/CommandGenerator.hpp"

namespace plato::third_party_integration::stk_io::test_utilities
{
/// @brief A base class fixture that creates a mesh using CommandGenerator on construction, and deletes it on
/// destruction.
class MeshGeneratingTestFixture : virtual public ::testing::Test
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
    constexpr static auto mExpectedNumberOfElements = 3U;
};

/// @brief A mesh creating test fixture that creates a 3x1x1 hex mesh with two nodesets on the x plus and minus faces.
class OneBlock3x1x1HexMeshWithNodeSets : public MeshGeneratingTestFixture
{
   protected:
    OneBlock3x1x1HexMeshWithNodeSets();
};

/// @brief A mesh test fixture that provides the path to a specific two-block mesh and has members
/// giving its node and element sizes.
class TwoBlockMeshOnDisk : virtual public ::testing::Test
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
class TwoDNonUniformHexMesh : virtual public ::testing::Test
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

/// @brief A mesh test fixture that provides the path to a mesh composed of tet4 elements.
class Tet4MeshOnDisk : virtual public ::testing::Test
{
   protected:
    Tet4MeshOnDisk();

    std::filesystem::path mMeshFilePath;

    constexpr static auto mMeshFileName = std::string_view{"box_3x4x7_tet4.cdf"};
    constexpr static auto mExpectedNumberOfNodes = 389U;
    constexpr static auto mExpectedNumberOfElements = 1520U;
};

/// @brief A mesh test fixture providing a 2D mesh with 3 blocks. The nodes and elements
/// are ordered so that the first nodes/elements do not belong to the first block.
class TwoDThreeBlockMesh : virtual public ::testing::Test
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

    constexpr static auto mBlockNames = std::array{"BLOCK_1", "BLOCK_2", "BLOCK_3"};
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

/// @brief A mesh test fixture providing a 3D tet mesh with 2 blocks.
class ThreeDTwoBlockTetMesh : virtual public ::testing::Test
{
   protected:
    ThreeDTwoBlockTetMesh();
    ~ThreeDTwoBlockTetMesh();

    std::filesystem::path mMeshFilePath = "three_d_two_block_tets.exo";

    constexpr static auto mMeshDescription = std::string_view{
        "textmesh:"
        "0,1,TET_4,5,1,2,3,block_1\n"
        "0,2,TET_4,6,5,2,3,block_1\n"
        "0,3,TET_4,6,7,5,3,block_1\n"
        "0,4,TET_4,6,4,7,3,block_1\n"
        "0,5,TET_4,6,2,4,3,block_1\n"
        "0,6,TET_4,6,8,7,4,block_1\n"
        "0,7,TET_4,9,5,6,7,block_2\n"
        "0,8,TET_4,10,9,6,7,block_2\n"
        "0,9,TET_4,10,11,9,7,block_2\n"
        "0,10,TET_4,10,8,11,7,block_2\n"
        "0,11,TET_4,10,6,8,7,block_2\n"
        "0,12,TET_4,10,12,11,8,block_2\n"
        "|coordinates: 0,-1,-1,0,0,-1,1,-1,-1,1,0,-1,0,-1,1,0,0,1,1,-1,1,1,0,1,0,-1,3,0,0,3,1,-1,3,1,0,3"
        "|dimension:3"};

    constexpr static auto mBlockNames = std::array{"BLOCK_1", "BLOCK_2"};
    constexpr static auto mExpectedNumberOfBlocks = 2U;
    constexpr static auto mExpectedNumberOfElements = 12U;
    constexpr static auto mExpectedNumberOfElementsInBlock1 = 6U;
    constexpr static auto mExpectedNumberOfElementsInBlock2 = 6U;
    constexpr static auto mExpectedNumberOfNodes = 12U;
    constexpr static auto mExpectedNumberOfNodesInBlock1 = 8U;
    constexpr static auto mExpectedNumberOfNodesInBlock2 = 8U;
};

/// @brief A mesh test fixture providing a 2D mesh with 2 blocks with non-standard block names.
class TwoDTwoBlockMesh : virtual public ::testing::Test
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

    constexpr static auto mBlockNames = std::array{"FIXED", "DESIGN"};
    constexpr static auto mExpectedNumberOfBlocks = 2U;
    constexpr static auto mExpectedNumberOfElements = 3U;
    constexpr static auto mExpectedNumberOfElementsInBlock1 = 1U;
    constexpr static auto mExpectedNumberOfElementsInBlock2 = 2U;
    constexpr static auto mExpectedNumberOfNodes = 6U;
    constexpr static auto mExpectedNumberOfNodesInBlock1 = 4U;
    constexpr static auto mExpectedNumberOfNodesInBlock2 = 4U;
};

/// @brief A 2D mesh with many blocks, useful for testing fixed blocks.
class TwoDManyBlockMesh : virtual public ::testing::Test
{
   protected:
    TwoDManyBlockMesh();
    ~TwoDManyBlockMesh();

    std::filesystem::path mMeshFilePath = "two_d_many_block.exo";

    constexpr static auto mMeshDescription = std::string_view{
        "textmesh:"
        "0,3,QUAD_4_2D,2,5,6,3,alpha\n"
        "0,1,TRI_3_2D,1,2,3,beta\n"
        "0,2,TRI_3_2D,3,4,1,gamma\n"
        "0,4,TRI_3_2D,2,7,9,delta\n"
        "0,5,TRI_3_2D,7,8,9,epsilon\n"
        "0,6,TRI_3_2D,8,5,9,zeta\n"
        "0,7,TRI_3_2D,5,2,9,eta\n"
        "|coordinates: -2,0,0,0,0,1,-2,1,2,0,2,1,0,-1,2,-1,1,-0.5"
        "|dimension:2"};

    constexpr static auto mBlockNames = std::array{"ALPHA", "BETA", "GAMMA", "DELTA", "EPSILON", "ZETA", "ETA"};
    constexpr static auto mExpectedNumberOfBlocks = 7U;
    constexpr static auto mExpectedNumberOfElements = 7U;
    constexpr static auto mExpectedNumberOfNodes = 9U;
    const static inline auto mBlockNameToOrdinal = std::unordered_map<std::string, unsigned int>{
        {"alpha", 20U}, {"beta", 21U}, {"gamma", 22U}, {"delta", 23U}, {"epsilon", 24U}, {"zeta", 25U}, {"eta", 26U}};
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

/// @brief A mesh test fixture providing a 3D mesh with coordinates only in the XZ plane.
class XZPlaneMesh : virtual public ::testing::Test
{
   protected:
    XZPlaneMesh();
    ~XZPlaneMesh();

    std::filesystem::path mMeshFilePath = "xz_plane_mesh.exo";

    constexpr static auto mMeshDescription = std::string_view{
        "textmesh:"
        "0,1,SHELL_TRI_3,1,2,3,design\n"
        "0,2,SHELL_TRI_3,1,3,4,design\n"
        "|coordinates: 0,0,0,  1,0,0,  1,0,1,  0,0,1"
        "|dimension:3"};

    constexpr static auto mBlockNames = std::array{"DESIGN"};
};

/// @brief A mesh test fixture providing a 3D mesh with coordinates only in a 30 degree wedge in the positive octant.
class ThirtyDegreeWedgeMesh : virtual public ::testing::Test
{
   protected:
    ThirtyDegreeWedgeMesh();
    ~ThirtyDegreeWedgeMesh();

    std::filesystem::path mMeshFilePath = "thirty_degree_wedge_mesh.exo";

    constexpr static auto mMeshDescription = std::string_view{
        "textmesh:"
        "0,1,HEX_8,1,2,3,4,5,6,7,8,design\n"
        "|coordinates: 0,0,0,  1,0,0,  2,0.5,0,  1,0.577350269189626,0,  0,0,1,  1,0,1,  2,0.5,1,  "
        "1,0.577350269189626,1"
        "|dimension:3"};

    constexpr static auto mBlockNames = std::array{"DESIGN"};
};

}  // namespace plato::third_party_integration::stk_io::test_utilities

#endif
