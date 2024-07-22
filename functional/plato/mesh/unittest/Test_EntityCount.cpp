#include <gtest/gtest.h>

#include "plato/mesh/EntityCounts.hpp"
#include "plato/mesh/Mesh.hpp"
#include "plato/mesh/unittest/Fixtures.hpp"
#include "plato/test_utilities/TestContext.hpp"

namespace plato::mesh::unittest
{
namespace
{
struct ExpectedCounts
{
    std::size_t mNumberOfElements = 0u;
    std::size_t mNumberOfNodes = 0u;
    std::size_t mNumberOfBlocks = 0u;
    std::size_t mSpatialDimensions = 0u;
};

void check_counts(const Mesh& aMesh, const ExpectedCounts& aCounts, const test_utilities::TestContext& aTestContext)
{
    const auto tMeshWithEntityCount = EntityCounts{aMesh};
    EXPECT_EQ(tMeshWithEntityCount.numberOfElements(), aCounts.mNumberOfElements) << aTestContext;
    EXPECT_EQ(tMeshWithEntityCount.numberOfNodes(), aCounts.mNumberOfNodes) << aTestContext;
    EXPECT_EQ(tMeshWithEntityCount.numberOfBlocks(), aCounts.mNumberOfBlocks) << aTestContext;
    EXPECT_EQ(tMeshWithEntityCount.spatialDimensions(), aCounts.mSpatialDimensions) << aTestContext;
}
}  // namespace

TEST_F(OneBlock3x1x1HexMesh, FilePath)
{
    const auto tMesh = Mesh{mMeshFilePath};
    EXPECT_EQ(tMesh.filePath(), mMeshFilePath);
}

TEST_F(OneBlock3x1x1HexMesh, OneBlockCounts)
{
    const auto tMesh = Mesh{mMeshFilePath};
    const auto tExpectedCounts = ExpectedCounts{/*.mNumberOfElements=*/mCommandGenerator.numberOfElements(),
                                                /*.mNumberOfNodes=*/mCommandGenerator.numberOfNodes(),
                                                /*.mNumberOfBlocks=*/1u, /*.mSpatialDimensions=*/3u};
    check_counts(tMesh, tExpectedCounts, TEST_CONTEXT("One block"));
}

TEST_F(TwoBlockMeshOnDisk, TwoBlockCounts)
{
    const auto tMesh = Mesh{mMeshFilePath};
    const auto tExpectedCounts =
        ExpectedCounts{/*.mNumberOfElements=*/mExpectedNumberOfElementsInBlock1 + mExpectedNumberOfElementsInBlock2,
                       /*.mNumberOfNodes=*/mExpectedNumberOfNodesInBlock1 + mExpectedNumberOfNodesInBlock2,
                       /*.mNumberOfBlocks=*/mExpectedNumberOfBlocks, /*.mSpatialDimensions=*/3u};
    check_counts(tMesh, tExpectedCounts, TEST_CONTEXT("Two blocks"));
}

TEST_F(TwoDNonUniformHexMesh, TwoDCounts)
{
    const auto tMesh = Mesh{mMeshFilePath};
    const auto tExpectedCounts =
        ExpectedCounts{/*.mNumberOfElements=*/mExpectedNumberOfElements,
                       /*.mNumberOfNodes=*/mExpectedNumberOfNodes,
                       /*.mNumberOfBlocks=*/mExpectedNumberOfBlocks, /*.mSpatialDimensions=*/2u};
    check_counts(tMesh, tExpectedCounts, TEST_CONTEXT("Two dimensional mesh"));
}

}  // namespace plato::mesh::unittest
