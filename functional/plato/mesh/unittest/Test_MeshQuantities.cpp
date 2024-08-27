#include <gtest/gtest.h>

#include <numeric>

#include "plato/mesh/Mesh.hpp"
#include "plato/mesh/MeshDesignVariablesDensitiesView.hpp"
#include "plato/mesh/MeshQuantities.hpp"
#include "plato/third_party_integration/stk_io/test_utilities/MeshFixtures.hpp"
#include "plato/utilities/Zip.hpp"

namespace plato::mesh::unittest
{
namespace
{
using third_party_integration::stk_io::test_utilities::OneBlock3x1x1HexMesh;
using third_party_integration::stk_io::test_utilities::TwoBlockMeshOnDisk;
using third_party_integration::stk_io::test_utilities::TwoDNonUniformHexMesh;
using third_party_integration::stk_io::test_utilities::TwoDThreeBlockMesh;

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
/*
TEST_F(OneBlock3x1x1HexMesh, MeshQuantitiesElementVolumes)
{
    const auto tMesh = MeshQuantities{Mesh{mMeshFilePath}};
    const auto tResultElementVolumes = tMesh.elementVolumes();
    const auto tExpectedElementVolume = double{1.0};
    for (const auto tResultElementVolume : tResultElementVolumes)
    {
        EXPECT_DOUBLE_EQ(tResultElementVolume, tExpectedElementVolume);
    }
}

TEST_F(TwoBlockMeshOnDisk, MeshQuantitiesElementVolumes)
{
    // Check that the element volumes are in the expected order.
    // There should be 273 tets first, then 40 hexes with volume 1.
    const auto tMesh = MeshQuantities{Mesh{mMeshFilePath}};
    const auto tResultElementVolumes = tMesh.elementVolumes();
    ASSERT_EQ(tResultElementVolumes.size(), mExpectedNumberOfElementsInBlock1 + mExpectedNumberOfElementsInBlock2);
    // Check that the sum of the tet volumes is 40:
    const auto tBlock1End = std::next(tResultElementVolumes.cbegin(), mExpectedNumberOfElementsInBlock1);
    const auto tResultElementVolumesBlock1 = std::reduce(tResultElementVolumes.cbegin(), tBlock1End);
    EXPECT_DOUBLE_EQ(tResultElementVolumesBlock1, mExpectedVolume / 2.0);
    // Check that all hex volumes are one
    const auto tBlock2Begin = std::next(tResultElementVolumes.cbegin(), mExpectedNumberOfElementsInBlock1);
    std::for_each(tBlock2Begin, tResultElementVolumes.cend(),
                  [](const double tHexVolume)
                  {
                      constexpr auto tExpectedHexVolume = double{1.0};
                      EXPECT_DOUBLE_EQ(tHexVolume, tExpectedHexVolume);
                  });
}

TEST_F(TwoDThreeBlockMesh, MeshQuantitiesElementVolumes)
{
    const auto tMesh = MeshQuantities{Mesh{mMeshFilePath}};
    const auto tResultElementVolumes = tMesh.elementVolumes();
    // Check exact order, should be in order of MeshDesignVariablesDensitiesView,
    // which is ordered by blocks first.
    constexpr auto tExpectedNumberOfElements =
        mExpectedNumberOfElementsInBlock1 + mExpectedNumberOfElementsInBlock2 + mExpectedNumberOfElementsInBlock3;
    ASSERT_EQ(tResultElementVolumes.size(), tExpectedNumberOfElements);

    // Block 1, four tris
    constexpr auto tExpectedElementVolumeBlock1 = double{0.5};
    EXPECT_DOUBLE_EQ(tResultElementVolumes[0], tExpectedElementVolumeBlock1);
    EXPECT_DOUBLE_EQ(tResultElementVolumes[1], tExpectedElementVolumeBlock1);
    EXPECT_DOUBLE_EQ(tResultElementVolumes[2], tExpectedElementVolumeBlock1);
    EXPECT_DOUBLE_EQ(tResultElementVolumes[3], tExpectedElementVolumeBlock1);

    // Block 2, two tris
    constexpr auto tExpectedElementVolumeBlock2 = double{1.0};
    EXPECT_DOUBLE_EQ(tResultElementVolumes[4], tExpectedElementVolumeBlock2);
    EXPECT_DOUBLE_EQ(tResultElementVolumes[5], tExpectedElementVolumeBlock2);

    // Block 3, single hex
    constexpr auto tExpectedElementVolumeBlock3 = double{2.0};
    EXPECT_DOUBLE_EQ(tResultElementVolumes[6], tExpectedElementVolumeBlock3);
}
*/
TEST_F(TwoDThreeBlockMesh, MeshQuantitiesDesignFixedElementVolumes)
{
    const auto tExpectedVolumesBlock1 =
        std::vector<double>(mExpectedNumberOfElementsInBlock1, kExpectedElementVolumeBlock1);
    const auto tExpectedVolumesBlock2 =
        std::vector<double>(mExpectedNumberOfElementsInBlock2, kExpectedElementVolumeBlock2);
    const auto tExpectedVolumesBlock3 =
        std::vector<double>(mExpectedNumberOfElementsInBlock3, kExpectedElementVolumeBlock3);

    {
        const auto tMesh = MeshQuantities{Mesh{mMeshFilePath, {"block_1"}}};
        EXPECT_EQ(tMesh.fixedDomainElementVolumes(), tExpectedVolumesBlock1);
    }
    {
        const auto tMesh = MeshQuantities{Mesh{mMeshFilePath, {"block_2"}}};
        EXPECT_EQ(tMesh.fixedDomainElementVolumes(), tExpectedVolumesBlock2);
    }
    {
        const auto tMesh = MeshQuantities{Mesh{mMeshFilePath, {"block_3"}}};
        EXPECT_EQ(tMesh.fixedDomainElementVolumes(), tExpectedVolumesBlock3);
    }
    {
        const auto tMesh = MeshQuantities{Mesh{mMeshFilePath, {"block_3", "block_2"}}};
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
        const auto tMesh = MeshQuantities{Mesh{mMeshFilePath, {"block_1", "block_2"}}};
        EXPECT_EQ(tMesh.designDomainElementVolumes(), tExpectedVolumesBlock3);
    }
    {
        const auto tMesh = MeshQuantities{Mesh{mMeshFilePath, {"block_3", "block_1"}}};
        EXPECT_EQ(tMesh.designDomainElementVolumes(), tExpectedVolumesBlock2);
    }
    {
        const auto tMesh = MeshQuantities{Mesh{mMeshFilePath, {"block_2", "block_3"}}};
        EXPECT_EQ(tMesh.designDomainElementVolumes(), tExpectedVolumesBlock1);
    }
    {
        const auto tMesh = MeshQuantities{Mesh{mMeshFilePath, {"block_3"}}};
        auto tExpectedElementVolumeBlocks1And2 = tExpectedVolumesBlock1;
        std::copy(tExpectedVolumesBlock2.cbegin(), tExpectedVolumesBlock2.cend(),
                  std::back_inserter(tExpectedElementVolumeBlocks1And2));
        EXPECT_EQ(tMesh.designDomainElementVolumes(), tExpectedElementVolumeBlocks1And2);
    }
}

}  // namespace plato::mesh::unittest
