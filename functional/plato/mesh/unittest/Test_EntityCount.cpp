#include <gtest/gtest.h>

#include "plato/design_variables/MeshDesignVariables.hpp"
#include "plato/mesh/EntityCounts.hpp"
#include "plato/mesh/Mesh.hpp"
#include "plato/test_utilities/TestContext.hpp"
#include "plato/third_party_integration/stk_io/test_utilities/MeshFixtures.hpp"

namespace plato::mesh::unittest
{
namespace
{
using third_party_integration::stk_io::test_utilities::OneBlock3x1x1HexMesh;
using third_party_integration::stk_io::test_utilities::TwoBlockMeshOnDisk;
using third_party_integration::stk_io::test_utilities::TwoDNonUniformHexMesh;
using third_party_integration::stk_io::test_utilities::TwoDThreeBlockMesh;

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
    if (aCounts.mSpatialDimensions == 2u)
    {
        EXPECT_TRUE(tMeshWithEntityCount.is2D()) << aTestContext;
        EXPECT_FALSE(tMeshWithEntityCount.is3D()) << aTestContext;
    }
    else
    {
        EXPECT_FALSE(tMeshWithEntityCount.is2D()) << aTestContext;
        EXPECT_TRUE(tMeshWithEntityCount.is3D()) << aTestContext;
    }
}
}  // namespace

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

TEST_F(TwoDThreeBlockMesh, TwoDThreeBlockCounts)
{
    const auto tMesh = Mesh{mMeshFilePath};
    const auto tExpectedCounts =
        ExpectedCounts{/*.mNumberOfElements=*/mExpectedNumberOfElementsInBlock1 + mExpectedNumberOfElementsInBlock2 +
                           mExpectedNumberOfElementsInBlock3,
                       /*.mNumberOfNodes=*/mExpectedNumberOfNodes,
                       /*.mNumberOfBlocks=*/mExpectedNumberOfBlocks, /*.mSpatialDimensions=*/2u};
    check_counts(tMesh, tExpectedCounts, TEST_CONTEXT("Two dimensional mesh with three blocks"));
}

TEST_F(TwoDThreeBlockMesh, TwoDThreeBlockDesignVariableCounts)
{
    const auto tMesh = Mesh{mMeshFilePath, {"block_2"}};
    constexpr auto tExpectedNumberOfElements = mExpectedNumberOfElementsInBlock1 + mExpectedNumberOfElementsInBlock3;
    const auto tNumberOfElementsInDesignDomain = EntityCounts{tMesh}.numberOfDesignDomainElements();
    EXPECT_EQ(tNumberOfElementsInDesignDomain, tExpectedNumberOfElements);

    constexpr auto tExpectedNumberOfNodes = 7u;
    const auto tNumberOfNodesInDesignDomain = EntityCounts{tMesh}.numberOfDesignDomainNodes();
    EXPECT_EQ(tNumberOfNodesInDesignDomain, tExpectedNumberOfNodes);
}

TEST_F(OneBlock3x1x1HexMesh, OneBlockHexAreDesignVariablesElementOrNodal)
{
    const auto setupDesignVariables =
        [this](const std::size_t aNumberOfEntities) -> std::pair<design_variables::MeshDesignVariables, EntityCounts>
    {
        const auto tFieldVector =
            std::vector<design_variables::ScalarFieldValue>(aNumberOfEntities, design_variables::ScalarFieldValue{});
        const auto tBlockField = design_variables::MeshDesignVariables::BlockScalarField{{1, tFieldVector}};
        const auto tDesignVariables = design_variables::MeshDesignVariables{mMeshFilePath, tBlockField};
        const auto tMesh = EntityCounts{Mesh{mMeshFilePath}};
        return {tDesignVariables, tMesh};
    };

    // Nodes
    {
        const auto [tDesignVariables, tMesh] = setupDesignVariables(mCommandGenerator.numberOfNodes());
        EXPECT_TRUE(tMesh.areNodalDesignVariables(tDesignVariables));
        EXPECT_FALSE(tMesh.areElementDesignVariables(tDesignVariables));
    }
    // Elements
    {
        const auto [tDesignVariables, tMesh] = setupDesignVariables(mCommandGenerator.numberOfElements());
        EXPECT_FALSE(tMesh.areNodalDesignVariables(tDesignVariables));
        EXPECT_TRUE(tMesh.areElementDesignVariables(tDesignVariables));
    }
}

TEST_F(TwoDThreeBlockMesh, TwoDThreeBlockAreNodalDesignVariables)
{
    const auto tFieldVectorBlock1 = std::vector<design_variables::ScalarFieldValue>{
        {2, 0, 0.0}, {5, 3, 0.0}, {7, 5, 0.0}, {8, 6, 0.0}, {9, 7, 0.0}};
    const auto tFieldVectorBlock3 =
        std::vector<design_variables::ScalarFieldValue>{{2, 0, 0.0}, {3, 1, 0.0}, {5, 3, 0.0}, {6, 4, 0.0}};
    const auto tBlockField =
        design_variables::MeshDesignVariables::BlockScalarField{{1, tFieldVectorBlock1}, {3, tFieldVectorBlock3}};
    const auto tDesignVariables = design_variables::MeshDesignVariables{mMeshFilePath, tBlockField};

    const auto tFixedBlocks = std::set<std::string>{"block_2"};
    const auto tMesh = EntityCounts{Mesh{mMeshFilePath, tFixedBlocks}};
    EXPECT_TRUE(tMesh.areNodalDesignVariables(tDesignVariables));
    EXPECT_FALSE(tMesh.areElementDesignVariables(tDesignVariables));
}

TEST_F(TwoDThreeBlockMesh, TwoDThreeBlockAreElementDesignVariables)
{
    const auto tFieldVectorBlock2 = std::vector<design_variables::ScalarFieldValue>{{1, 0, 0.0}, {2, 1, 0.0}};
    const auto tFieldVectorBlock3 = std::vector<design_variables::ScalarFieldValue>{{3, 2, 0.0}};
    const auto tBlockField =
        design_variables::MeshDesignVariables::BlockScalarField{{2, tFieldVectorBlock2}, {3, tFieldVectorBlock3}};
    const auto tDesignVariables = design_variables::MeshDesignVariables{mMeshFilePath, tBlockField};

    const auto tFixedBlocks = std::set<std::string>{"block_1"};
    const auto tMesh = EntityCounts{Mesh{mMeshFilePath, tFixedBlocks}};
    EXPECT_FALSE(tMesh.areNodalDesignVariables(tDesignVariables));
    EXPECT_TRUE(tMesh.areElementDesignVariables(tDesignVariables));
}

}  // namespace plato::mesh::unittest
