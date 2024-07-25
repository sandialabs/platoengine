#include <gtest/gtest.h>

#include <numeric>

#include "plato/mesh/DesignVariableConversion.hpp"
#include "plato/mesh/EntityCounts.hpp"
#include "plato/mesh/Mesh.hpp"
#include "plato/test_utilities/TestContext.hpp"
#include "plato/third_party_integration/stk_io/test_utilities/Fixtures.hpp"

namespace plato::mesh::unittest
{
namespace
{
using third_party_integration::stk_io::test_utilities::TwoBlockMeshOnDisk;

void check_two_block_properties(const MeshProxy& aMeshProxy, const test_utilities::TestContext& aTestContext)
{
    for (const auto& [tBlockID, tDensities] : aMeshProxy.mBlockDensities)
    {
        for (const auto& tResult : tDensities)
        {
            EXPECT_EQ(tResult.mDensity, tResult.mGlobalID) << aTestContext;
        }
    }
}

}  // namespace

TEST_F(TwoBlockMeshOnDisk, NodalDensitiesToMeshProxy)
{
    const auto tMesh = Mesh{mMeshFilePath};
    auto tDensities = std::vector<double>(EntityCounts{tMesh}.numberOfNodes());
    std::iota(tDensities.begin(), tDensities.end(), 1.0);
    const auto tMeshProxy = nodal_densities_to_mesh_proxy(tDensities, tMesh);

    ASSERT_EQ(tMeshProxy.mBlockDensities.size(), mExpectedNumberOfBlocks);
    EXPECT_EQ(tMeshProxy.mBlockDensities.cbegin()->second.size(), mExpectedNumberOfNodesInBlock1);
    EXPECT_EQ(std::next(tMeshProxy.mBlockDensities.cbegin())->second.size(), mExpectedNumberOfNodesInBlock2);

    check_two_block_properties(tMeshProxy, TEST_CONTEXT("Nodal densities"));
}

TEST_F(TwoBlockMeshOnDisk, ElementDensitiesToMeshProxy)
{
    const auto tMesh = Mesh{mMeshFilePath};
    auto tDensities = std::vector<double>(EntityCounts{tMesh}.numberOfElements(), 1.0);
    std::iota(tDensities.begin(), tDensities.end(), 1.0);
    const auto tMeshProxy = element_densities_to_mesh_proxy(tDensities, tMesh);

    ASSERT_EQ(tMeshProxy.mBlockDensities.size(), mExpectedNumberOfBlocks);
    EXPECT_EQ(tMeshProxy.mBlockDensities.cbegin()->second.size(), mExpectedNumberOfElementsInBlock1);
    EXPECT_EQ(std::next(tMeshProxy.mBlockDensities.cbegin())->second.size(), mExpectedNumberOfElementsInBlock2);

    check_two_block_properties(tMeshProxy, TEST_CONTEXT("Element densities"));
}

}  // namespace plato::mesh::unittest
