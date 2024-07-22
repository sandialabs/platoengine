#include <gtest/gtest.h>

#include "plato/mesh/EntityCounts.hpp"
#include "plato/mesh/Mesh.hpp"
#include "plato/mesh/unittest/Fixtures.hpp"

namespace plato::mesh::unittest
{
class EntityCountMeshFixture : public OneBlock3x1x1HexMesh
{
};

TEST_F(EntityCountMeshFixture, OneBlockCounts)
{
    const auto tMesh = Mesh{mMeshFilePath};
    const auto tMeshWithEntityCount = EntityCounts{tMesh};

    EXPECT_EQ(tMeshWithEntityCount.numberOfElements(), mCommandGenerator.numberOfElements());
    EXPECT_EQ(tMeshWithEntityCount.numberOfNodes(), mCommandGenerator.numberOfNodes());
    constexpr auto tExpectedNumberOfBlocks = 1u;
    EXPECT_EQ(tMeshWithEntityCount.numberOfBlocks(), tExpectedNumberOfBlocks);
    constexpr auto tSpatialDimensions = unsigned{3};
    EXPECT_EQ(tMeshWithEntityCount.spatialDimensions(), tSpatialDimensions);
    EXPECT_EQ(tMeshWithEntityCount.filePath(), mMeshFilePath);
}

TEST_F(TwoBlockMeshOnDisk, TwoBlockCounts)
{
    const auto tMesh = Mesh{mMeshFilePath};
    const auto tMeshWithEntityCount = EntityCounts{tMesh};

    EXPECT_EQ(tMeshWithEntityCount.numberOfElements(),
              mExpectedNumberOfElementsInBlock1 + mExpectedNumberOfElementsInBlock2);
    EXPECT_EQ(tMeshWithEntityCount.numberOfNodes(), mExpectedNumberOfNodesInBlock1 + mExpectedNumberOfNodesInBlock2);
    constexpr auto tExpectedNumberOfBlocks = 2u;
    EXPECT_EQ(tMeshWithEntityCount.numberOfBlocks(), tExpectedNumberOfBlocks);
    constexpr auto tSpatialDimensions = unsigned{3};
    EXPECT_EQ(tMeshWithEntityCount.spatialDimensions(), tSpatialDimensions);
    EXPECT_EQ(tMeshWithEntityCount.filePath(), mMeshFilePath);
}

}  // namespace plato::mesh::unittest
