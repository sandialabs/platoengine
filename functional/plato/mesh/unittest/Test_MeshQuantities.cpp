#include <gtest/gtest.h>

#include <numeric>

#include "plato/mesh/Mesh.hpp"
#include "plato/mesh/MeshDesignVariablesViews.hpp"
#include "plato/mesh/MeshQuantities.hpp"
#include "plato/third_party_integration/stk_io/test_utilities/MeshFixtures.hpp"
#include "plato/utilities/Zip.hpp"

namespace plato::mesh::unittest
{
using third_party_integration::stk_io::test_utilities::OneBlock3x1x1HexMesh;
using third_party_integration::stk_io::test_utilities::TwoBlockMeshOnDisk;
using third_party_integration::stk_io::test_utilities::TwoDNonUniformHexMesh;
using third_party_integration::stk_io::test_utilities::TwoDThreeBlockMesh;

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

}  // namespace plato::mesh::unittest
