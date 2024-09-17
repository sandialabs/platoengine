#include <gtest/gtest.h>

#include <numeric>

#include "plato/analysis/AnalysisDomainMeshSequentialView.hpp"
#include "plato/mesh/Mesh.hpp"
#include "plato/mesh/MeshQuantities.hpp"
#include "plato/third_party_integration/stk_io/test_utilities/MeshFixtures.hpp"
#include "plato/utilities/Exception.hpp"
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

TEST_F(OneBlock3x1x1HexMesh, MeshQuantitiesSmallestElementVolume)
{
    const auto tMesh = MeshQuantities{Mesh{mMeshFilePath}};
    EXPECT_DOUBLE_EQ(tMesh.smallestDesignDomainElementVolume(), mCommandGenerator.volume() / 3.0);
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

TEST_F(TwoBlockMeshOnDisk, MeshQuantitiesSmallestElementVolume)
{
    const auto tMesh = MeshQuantities{Mesh{mMeshFilePath}};
    EXPECT_DOUBLE_EQ(tMesh.smallestDesignDomainElementVolume(), mSmallestElementVolume);
}

TEST_F(TwoDNonUniformHexMesh, MeshQuantitiesVolume)
{
    const auto tMesh = MeshQuantities{Mesh{mMeshFilePath}};
    EXPECT_DOUBLE_EQ(tMesh.volume(), mExpectedArea);
}

TEST_F(TwoDNonUniformHexMesh, MeshQuantitiesSmallestElementVolume)
{
    const auto tMesh = MeshQuantities{Mesh{mMeshFilePath}};
    EXPECT_DOUBLE_EQ(tMesh.smallestDesignDomainElementVolume(), mExpectedSmallestElementArea);
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
