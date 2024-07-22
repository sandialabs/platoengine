#include <gtest/gtest.h>

#include "plato/mesh/EntityRetrieval.hpp"
#include "plato/mesh/Mesh.hpp"
#include "plato/mesh/unittest/Fixtures.hpp"
#include "plato/test_utilities/TestContext.hpp"

namespace plato::mesh::unittest
{
namespace
{
void check_mesh_counts(const std::filesystem::path& aMeshFilePath,
                       const unsigned int aNumberOfNodes,
                       const unsigned int aNumberOfElements,
                       const test_utilities::TestContext& aTestContext)
{
    const auto tMeshBase = Mesh{aMeshFilePath};
    const auto tMesh = EntityRetrieval{tMeshBase};

    const auto tNodes = tMesh.nodalCoordinates();
    EXPECT_EQ(tNodes.size(), aNumberOfNodes) << aTestContext;

    const auto tCentroids = tMesh.elementCentroids();
    EXPECT_EQ(tCentroids.size(), aNumberOfElements) << aTestContext;

    const auto tFlattenCoordinates = tMesh.flattenedNodalCoordinates();
    constexpr auto tSpatialDimensions = unsigned{3};
    EXPECT_EQ(tFlattenCoordinates.size(), aNumberOfNodes * tSpatialDimensions) << aTestContext;
}
}  // namespace

TEST_F(OneBlock3x1x1HexMesh, OneBlockRetrieval)
{
    check_mesh_counts(mMeshFilePath, mCommandGenerator.numberOfNodes(), mCommandGenerator.numberOfElements(),
                      TEST_CONTEXT("One block mesh"));
}

TEST_F(TwoBlockMeshOnDisk, TwoBlockRetrieval)
{
    constexpr auto tTotalNumberOfNodes = mExpectedNumberOfNodesInBlock1 + mExpectedNumberOfNodesInBlock2;
    constexpr auto tTotalNumberOfElements = mExpectedNumberOfElementsInBlock1 + mExpectedNumberOfElementsInBlock2;
    check_mesh_counts(mMeshFilePath, tTotalNumberOfNodes, tTotalNumberOfElements, TEST_CONTEXT("Two block mesh"));
}

}  // namespace plato::mesh::unittest
