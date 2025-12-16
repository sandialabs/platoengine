#include <gtest/gtest.h>

#include <ranges>

#include "plato/mesh/Mesh.hpp"
#include "plato/mesh/MeshQuantities.hpp"
#include "plato/third_party_integration/stk_io/test_utilities/MeshFixtures.hpp"

namespace plato::mesh::unittest
{
namespace
{
using third_party_integration::stk_io::test_utilities::OneBlock3x1x1HexMesh;
using third_party_integration::stk_io::test_utilities::TwoBlockMeshOnDisk;
using third_party_integration::stk_io::test_utilities::TwoDNonUniformHexMesh;
using third_party_integration::stk_io::test_utilities::TwoDThreeBlockMesh;
using MeshQuantitiesThreeDTwoBlockTetMesh = third_party_integration::stk_io::test_utilities::ThreeDTwoBlockTetMesh;

constexpr auto kExpectedElementVolumeBlock1 = double{0.5};
constexpr auto kExpectedElementVolumeBlock2 = double{1.0};
constexpr auto kExpectedElementVolumeBlock3 = double{2.0};
}  // namespace

TEST_F(OneBlock3x1x1HexMesh, MeshQuantitiesVolume)
{
    const auto tMesh = MeshQuantities{Mesh{mMeshFilePath}};
    EXPECT_DOUBLE_EQ(tMesh.volume(), mCommandGenerator.volume());
}

TEST_F(OneBlock3x1x1HexMesh, MeshQuantitiesNodalDensity)
{
    const auto tMesh = MeshQuantities{Mesh{mMeshFilePath}};
    const auto tTotalNumberOfNodes = mCommandGenerator.numberOfNodes();
    const auto tTotalVolume = mCommandGenerator.volume();
    const auto tExpectedNodalDensity = static_cast<double>(tTotalNumberOfNodes) / tTotalVolume;
    EXPECT_DOUBLE_EQ(tMesh.averageNodalDensity(), tExpectedNodalDensity);
}

TEST_F(TwoBlockMeshOnDisk, MeshQuantitiesVolume)
{
    const auto tMesh = MeshQuantities{Mesh{mMeshFilePath}};
    EXPECT_DOUBLE_EQ(tMesh.volume(), mExpectedVolume);
}

TEST_F(TwoDNonUniformHexMesh, MeshQuantitiesVolume)
{
    const auto tMesh = MeshQuantities{Mesh{mMeshFilePath}};
    EXPECT_DOUBLE_EQ(tMesh.volume(), mExpectedArea);
}

TEST_F(TwoDThreeBlockMesh, MeshQuantitiesFixedDomainElementVolumes)
{
    const auto tExpectedVolumesBlock1 =
        std::vector<double>(mExpectedNumberOfElementsInBlock1, kExpectedElementVolumeBlock1);
    const auto tExpectedVolumesBlock2 =
        std::vector<double>(mExpectedNumberOfElementsInBlock2, kExpectedElementVolumeBlock2);
    const auto tExpectedVolumesBlock3 =
        std::vector<double>(mExpectedNumberOfElementsInBlock3, kExpectedElementVolumeBlock3);

    {
        const auto tMesh = MeshQuantities{Mesh{mMeshFilePath, {mBlockNames[0]}}};
        EXPECT_EQ(tMesh.fixedDomainElementVolumes(), tExpectedVolumesBlock1);
    }
    {
        const auto tMesh = MeshQuantities{Mesh{mMeshFilePath, {mBlockNames[1]}}};
        EXPECT_EQ(tMesh.fixedDomainElementVolumes(), tExpectedVolumesBlock2);
    }
    {
        const auto tMesh = MeshQuantities{Mesh{mMeshFilePath, {mBlockNames[2]}}};
        EXPECT_EQ(tMesh.fixedDomainElementVolumes(), tExpectedVolumesBlock3);
    }
    {
        const auto tMesh = MeshQuantities{Mesh{mMeshFilePath, {mBlockNames[2], mBlockNames[1]}}};
        auto tExpectedElementVolumeBlocks2And3 = tExpectedVolumesBlock2;
        std::copy(tExpectedVolumesBlock3.cbegin(), tExpectedVolumesBlock3.cend(),
                  std::back_inserter(tExpectedElementVolumeBlocks2And3));
        EXPECT_EQ(tMesh.fixedDomainElementVolumes(), tExpectedElementVolumeBlocks2And3);
    }
}

TEST_F(TwoDThreeBlockMesh, MeshQuantitiesDesignDomainElementVolumes)
{
    const auto tExpectedVolumesBlock1 =
        std::vector<double>(mExpectedNumberOfElementsInBlock1, kExpectedElementVolumeBlock1);
    const auto tExpectedVolumesBlock2 =
        std::vector<double>(mExpectedNumberOfElementsInBlock2, kExpectedElementVolumeBlock2);
    const auto tExpectedVolumesBlock3 =
        std::vector<double>(mExpectedNumberOfElementsInBlock3, kExpectedElementVolumeBlock3);

    {
        const auto tMesh = MeshQuantities{Mesh{mMeshFilePath, {mBlockNames[0], mBlockNames[1]}}};
        EXPECT_EQ(tMesh.designDomainElementVolumes(), tExpectedVolumesBlock3);
    }
    {
        const auto tMesh = MeshQuantities{Mesh{mMeshFilePath, {mBlockNames[2], mBlockNames[0]}}};
        EXPECT_EQ(tMesh.designDomainElementVolumes(), tExpectedVolumesBlock2);
    }
    {
        const auto tMesh = MeshQuantities{Mesh{mMeshFilePath, {mBlockNames[1], mBlockNames[2]}}};
        EXPECT_EQ(tMesh.designDomainElementVolumes(), tExpectedVolumesBlock1);
    }
    {
        const auto tMesh = MeshQuantities{Mesh{mMeshFilePath, {mBlockNames[2]}}};
        auto tExpectedElementVolumeBlocks1And2 = tExpectedVolumesBlock1;
        std::copy(tExpectedVolumesBlock2.cbegin(), tExpectedVolumesBlock2.cend(),
                  std::back_inserter(tExpectedElementVolumeBlocks1And2));
        EXPECT_EQ(tMesh.designDomainElementVolumes(), tExpectedElementVolumeBlocks1And2);
    }
}

TEST_F(OneBlock3x1x1HexMesh, MeshQuantitiesNodalAverage)
{
    const auto tMesh = MeshQuantities{Mesh{mMeshFilePath}};
    const auto tNumberOfNodes = mCommandGenerator.numberOfNodes();
    auto tFieldEntries = std::views::iota(0U, tNumberOfNodes);  // Matches the nodal connectivity, minus 1
    const auto tField = std::vector<double>(tFieldEntries.begin(), tFieldEntries.end());

    const auto tNodalAverage = tMesh.elementAveragedNodalValues(tField);
    const auto tExpected = std::vector{6.5, 7.5, 8.5};  // Computed via the nodal connectivity
    EXPECT_EQ(tNodalAverage, tExpected);
}

TEST_F(MeshQuantitiesThreeDTwoBlockTetMesh, MeshQuantitiesNodalAverageElementProjection)
{
    /* Computed via matlab:
     tets = [5,1,2,3
             6,5,2,3
             6,7,5,3
             6,4,7,3
             6,2,4,3
             6,8,7,4
             9,5,6,7
             10,9,6,7
             10,11,9,7
             10,8,11,7
             10,6,8,7
             10,12,11,8];
     field = 1:length(tets);
     coords = [0,-1,-1,0,0,-1,1,-1,-1,1,0,-1,0,-1,1,0,0,1,1,-1,1,1,0,1,0,-1,3,0,0,3,1,-1,3,1,0,3];
     nodal_projection = zeros(1,length(coords)/3)

     for ktet = 1:length(tets)
       num_nodes = length(tets(ktet, :));
       for knode = 1:num_nodes
         nodal_projection(tets(ktet, knode)) = nodal_projection(tets(ktet, knode)) + field(ktet) / num_nodes;
       end
     end
     nodal_projection
    */

    const auto tMesh = MeshQuantities{Mesh{mMeshFilePath}};
    auto tFieldEntries = std::views::iota(1U, mExpectedNumberOfElements + 1U);
    const auto tField = std::vector<double>(tFieldEntries.begin(), tFieldEntries.end());

    const auto tNodalAverage = tMesh.nodalAverageElementProjection(tField);
    const auto tExpected = std::vector{0.25, 2.0, 3.75, 3.75, 3.25, 11.5, 14.5, 9.75, 6.0, 12.5, 7.75, 3.0};
    EXPECT_EQ(tNodalAverage, tExpected);
}

}  // namespace plato::mesh::unittest
