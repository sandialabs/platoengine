#include <gtest/gtest.h>

#include <numeric>

#include "plato/mesh/EntityRetrieval.hpp"
#include "plato/mesh/Mesh.hpp"
#include "plato/test_utilities/TestContext.hpp"
#include "plato/third_party_integration/common/test_utilities/CoordinateTestUtilities.hpp"
#include "plato/third_party_integration/stk_io/test_utilities/MeshFixtures.hpp"
#include "plato/third_party_integration/stk_io/test_utilities/MeshWithFieldWriter.hpp"

namespace plato::mesh::unittest
{
namespace
{

using third_party_integration::stk_io::test_utilities::MeshWithNodalDensities;
using third_party_integration::stk_io::test_utilities::OneBlock3x1x1HexMesh;
using EntityRetrievalThreeDTwoBlockTetMesh = third_party_integration::stk_io::test_utilities::ThreeDTwoBlockTetMesh;
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
    const auto tMesh = Mesh{mMeshFilePath, {mBlockNames[0]}};
    const auto tDesignDomainNodes = EntityRetrieval{tMesh}.designDomainNodalCoordinates();

    const auto tExpectedNodalCoordinates = std::vector<third_party_integration::common::Coordinate>{
        {-2.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 1.0, 0.0}, {-2.0, 1.0, 0.0}, {2.0, 0.0, 0.0}, {2.0, 1.0, 0.0}};

    EXPECT_EQ(tExpectedNodalCoordinates, tDesignDomainNodes);
}

TEST_F(TwoDThreeBlockMesh, DesignDomainNodalIDsWithFixedBlocks)
{
    const auto tMesh = Mesh{mMeshFilePath, {mBlockNames[0]}};
    const auto tDesignDomainNodeIDs = EntityRetrieval{tMesh}.designDomainNodeIDs();
    const auto tExpectedNodalIDs = std::vector<std::size_t>{1U, 2U, 3U, 4U, 5U, 6U};
    EXPECT_EQ(tExpectedNodalIDs, tDesignDomainNodeIDs);
}

TEST_F(TwoDThreeBlockMesh, DesignDomainNodalIDsNoFixedBlocks)
{
    const auto tMesh = Mesh{mMeshFilePath, {}};
    const auto tDesignDomainNodeIDs = EntityRetrieval{tMesh}.designDomainNodeIDs();
    const auto tExpectedNodalIDs = std::vector<std::size_t>{1U, 2U, 3U, 4U, 5U, 6U, 7U, 8U, 9U};
    EXPECT_EQ(tExpectedNodalIDs, tDesignDomainNodeIDs);
}

TEST_F(TwoDThreeBlockMesh, AllNodalIDsWithFixedBlocks)
{
    const auto tMesh = Mesh{mMeshFilePath, {mBlockNames[0]}};
    const auto tDesignDomainNodeIDs = EntityRetrieval{tMesh}.allNodeIDs();
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
    const auto tMesh = Mesh{mMeshFilePath, {mBlockNames[0], mBlockNames[1]}};

    const auto tDesignDomainNodes = EntityRetrieval{tMesh}.designDomainElementCentroids();
    const auto tExpectedCoordinates = std::vector<third_party_integration::common::Coordinate>{{1.0, 0.5, 0.0}};

    EXPECT_EQ(tDesignDomainNodes, tExpectedCoordinates);
}

TEST_F(MeshWithNodalDensities, DesignDomainNodalField)
{
    const auto tCheckField = [this](const std::vector<double>& aResult, const test_utilities::TestContext& aTestContext)
    {
        EXPECT_EQ(aResult.size(), mGoldNumbering.size()) << aTestContext;
        std::vector<double> tGoldVector(mGoldNumbering.size());
        std::iota(tGoldVector.begin(), tGoldVector.end(), 1.0);
        EXPECT_EQ(tGoldVector, aResult) << aTestContext;
    };

    const auto tMesh = EntityRetrieval{Mesh{mMeshName, {}}};
    {
        const auto tResult = tMesh.designDomainNodalField(mFieldName);
        tCheckField(tResult, TEST_CONTEXT("No time step argument"));
    }
    {
        const auto tResult = tMesh.designDomainNodalField(mFieldName, 1.0);
        tCheckField(tResult, TEST_CONTEXT("Time step argument as 1.0"));
    }
    {
        const auto tResult = tMesh.designDomainNodalField(mFieldName, mesh::LastTimeStep{});
        tCheckField(tResult, TEST_CONTEXT("Time step argument as LastStep"));
    }
}

TEST_F(MeshWithNodalDensities, NodalFields)
{
    const auto tMesh = EntityRetrieval{Mesh{mMeshName, {}}};
    const auto tResult = tMesh.nodalFields();
    const auto tGold = std::vector<std::string>{"coordinates", "Topology"};
    EXPECT_EQ(tGold, tResult);
}

namespace
{

const auto kCheckBounds = [](const auto& aResult, const auto& aGold, const test_utilities::TestContext& aTestContext)
{
    third_party_integration::common::test_utilities::test_double_equality_of_components(aResult.first, aGold.first,
                                                                                        aTestContext);
    third_party_integration::common::test_utilities::test_double_equality_of_components(aResult.second, aGold.second,
                                                                                        aTestContext);
};

}  // namespace

TEST_F(TwoDThreeBlockMesh, DesignDomainBoundingBox)
{
    const auto tMesh = EntityRetrieval{Mesh{mMeshFilePath}};
    const auto tBounds = tMesh.designDomainBoundingBox();
    const auto tBoundsGold = std::make_pair(third_party_integration::common::Coordinate{-2, -1, 0},
                                            third_party_integration::common::Coordinate{2, 1, 0});
    kCheckBounds(tBounds, tBoundsGold, TEST_CONTEXT("Checking bounds on entire TwoDThreeBlockMesh"));
}

TEST_F(TwoDThreeBlockMesh, DesignDomainBoundingBoxWithFixedBlocks)
{
    const auto tMesh = EntityRetrieval{Mesh{mMeshFilePath, {mBlockNames[0]}}};
    const auto tBounds = tMesh.designDomainBoundingBox();
    const auto tBoundsGold = std::make_pair(third_party_integration::common::Coordinate{-2, 0, 0},
                                            third_party_integration::common::Coordinate{2, 1, 0});
    kCheckBounds(tBounds, tBoundsGold, TEST_CONTEXT("Checking bounds with block_1 fixed in TwoDThreeBlockMesh"));
}

TEST_F(EntityRetrievalThreeDTwoBlockTetMesh, DesignDomainBoundingBox)
{
    const auto tMesh = EntityRetrieval{Mesh{mMeshFilePath}};
    const auto tBounds = tMesh.designDomainBoundingBox();
    const auto tBoundsGold = std::make_pair(third_party_integration::common::Coordinate{0, -1, -1},
                                            third_party_integration::common::Coordinate{1, 0, 3});
    kCheckBounds(tBounds, tBoundsGold, TEST_CONTEXT("Checking bounds with block_1 fixed in ThreeDTwoBlockTetMesh"));
}

TEST_F(EntityRetrievalThreeDTwoBlockTetMesh, DesignDomainBoundingBoxWithFixedBlocks)
{
    const auto tMesh = EntityRetrieval{Mesh{mMeshFilePath, {mBlockNames[1]}}};
    const auto tBounds = tMesh.designDomainBoundingBox();
    const auto tBoundsGold = std::make_pair(third_party_integration::common::Coordinate{0, -1, -1},
                                            third_party_integration::common::Coordinate{1, 0, 1});
    kCheckBounds(tBounds, tBoundsGold, TEST_CONTEXT("Checking bounds with block_2 fixed in ThreeDTwoBlockTetMesh"));
}
}  // namespace plato::mesh::unittest
