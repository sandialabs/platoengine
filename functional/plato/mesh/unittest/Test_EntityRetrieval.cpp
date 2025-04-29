#include <gtest/gtest.h>

#include <numeric>

#include "plato/mesh/EntityRetrieval.hpp"
#include "plato/mesh/Mesh.hpp"
#include "plato/test_utilities/TestContext.hpp"
#include "plato/third_party_integration/stk_io/test_utilities/MeshFixtures.hpp"
#include "plato/third_party_integration/stk_io/test_utilities/MeshWithFieldWriter.hpp"

namespace plato::mesh::unittest
{
namespace
{

using third_party_integration::stk_io::test_utilities::MeshWithNodalDensities;
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

TEST_F(TwoDThreeBlockMesh, NodalIDsWithFixedBlocks)
{
    const auto tMesh = Mesh{mMeshFilePath, {"block_1"}};
    const auto tDesignDomainNodeIDs = EntityRetrieval{tMesh}.designDomainNodeIDs();
    const auto tExpectedNodalIDs = std::vector<std::size_t>{1U, 2U, 3U, 4U, 5U, 6U};
    EXPECT_EQ(tExpectedNodalIDs, tDesignDomainNodeIDs);
}

TEST_F(TwoDThreeBlockMesh, NodalIDs)
{
    const auto tMesh = Mesh{mMeshFilePath, {}};
    const auto tDesignDomainNodeIDs = EntityRetrieval{tMesh}.designDomainNodeIDs();
    const auto tExpectedNodalIDs = std::vector<std::size_t>{1U, 2U, 3U, 4U, 5U, 6U, 7U, 8U, 9U};
    EXPECT_EQ(tExpectedNodalIDs, tDesignDomainNodeIDs);
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

TEST_F(MeshWithNodalDensities, DesignDomainNodalField)
{
    const auto tMesh = EntityRetrieval{Mesh{mMeshName, {}}};
    const auto tResult = tMesh.designDomainNodalField(mFieldName);

    EXPECT_EQ(tResult.size(), mGoldNumbering.size());
    std::vector<double> tGoldVector(mGoldNumbering.size());
    std::iota(tGoldVector.begin(), tGoldVector.end(), 1.0);
    EXPECT_EQ(tGoldVector, tResult);
}

TEST_F(MeshWithNodalDensities, NodalFields)
{
    const auto tMesh = EntityRetrieval{Mesh{mMeshName, {}}};
    const auto tResult = tMesh.nodalFields();
    const auto tGold = std::vector<std::string>{"coordinates", "topology"};
    EXPECT_EQ(tGold, tResult);
}

TEST_F(MeshWithNodalDensities, HasNodalField)
{
    const auto tMesh = EntityRetrieval{Mesh{mMeshName, {}}};
    EXPECT_TRUE(tMesh.hasNodalField("coordinates"));
    EXPECT_TRUE(tMesh.hasNodalField("topology"));
    EXPECT_FALSE(tMesh.hasNodalField("not-a-hotdog"));
}

}  // namespace plato::mesh::unittest
