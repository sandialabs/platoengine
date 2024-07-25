#include <gtest/gtest.h>

#include "plato/mesh/EntityRetrieval.hpp"
#include "plato/mesh/Mesh.hpp"
#include "plato/test_utilities/TestContext.hpp"
#include "plato/third_party_integration/stk_io/test_utilities/MeshFixtures.hpp"

namespace plato::mesh::unittest
{
namespace
{
using third_party_integration::stk_io::test_utilities::OneBlock3x1x1HexMesh;
using third_party_integration::stk_io::test_utilities::TwoBlockMeshOnDisk;
using third_party_integration::stk_io::test_utilities::TwoDThreeBlockMesh;

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

TEST_F(TwoDThreeBlockMesh, NodalCoordinatesWithFixedBlocks)
{
    const auto tMesh = Mesh{mMeshFilePath, {"block_1"}};
    const auto tDesignDomainNodes = EntityRetrieval{tMesh}.designDomainNodalCoordinates();

    const auto tExpectedNodalCoordinates = std::vector<third_party_integration::common::Coordinate>{
        {-2.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 1.0, 0.0}, {-2.0, 1.0, 0.0}, {2.0, 0.0, 0.0}, {2.0, 1.0, 0.0}};

    EXPECT_EQ(tExpectedNodalCoordinates, tDesignDomainNodes);
}

TEST_F(TwoDThreeBlockMesh, ElementCentroidsDesignDomainSameAsFullMesh)
{
    const auto tMesh = Mesh{mMeshFilePath};

    const auto tDesignDomainNodes = EntityRetrieval{tMesh}.designDomainElementCentroids();
    const auto tFullMeshNodes = EntityRetrieval{tMesh}.elementCentroids();

    EXPECT_EQ(tDesignDomainNodes, tFullMeshNodes);
}

TEST_F(TwoDThreeBlockMesh, ElementCentroidsWithFixedBlock)
{
    const auto tMesh = Mesh{mMeshFilePath, {"block_1", "block_2"}};

    const auto tDesignDomainNodes = EntityRetrieval{tMesh}.designDomainElementCentroids();
    const auto tExpectedCoordinates = std::vector<third_party_integration::common::Coordinate>{{1.0, 0.5, 0.0}};

    EXPECT_EQ(tDesignDomainNodes, tExpectedCoordinates);
}

}  // namespace plato::mesh::unittest
