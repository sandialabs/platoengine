#include <gtest/gtest.h>

#include <numeric>

#include "plato/mesh/DesignVariableConversion.hpp"
#include "plato/mesh/EntityCounts.hpp"
#include "plato/mesh/Mesh.hpp"
#include "plato/test_utilities/TestContext.hpp"
#include "plato/third_party_integration/stk_io/test_utilities/MeshFixtures.hpp"

namespace plato::mesh::unittest
{
namespace
{
using third_party_integration::stk_io::test_utilities::TwoBlockMeshOnDisk;

void check_two_block_properties(const MeshDesignVariables& aMeshDesignVariables,
                                const test_utilities::TestContext& aTestContext)
{
    for (const auto& [tBlockID, tDensities] : aMeshDesignVariables.mBlockDensities)
    {
        for (const auto& tResult : tDensities)
        {
            EXPECT_EQ(tResult.mDensity, tResult.mGlobalID) << aTestContext;
        }
    }
}

}  // namespace

TEST_F(TwoBlockMeshOnDisk, NodalDensitiesToMeshDesignVariables)
{
    const auto tMesh = Mesh{mMeshFilePath};
    auto tDensities = std::vector<double>(EntityCounts{tMesh}.numberOfNodes());
    std::iota(tDensities.begin(), tDensities.end(), 1.0);
    const auto tMeshDesignVariables = nodal_densities_to_mesh_design_variables(tDensities, tMesh);

    ASSERT_EQ(tMeshDesignVariables.mBlockDensities.size(), mExpectedNumberOfBlocks);
    EXPECT_EQ(tMeshDesignVariables.mBlockDensities.cbegin()->second.size(), mExpectedNumberOfNodesInBlock1);
    EXPECT_EQ(std::next(tMeshDesignVariables.mBlockDensities.cbegin())->second.size(), mExpectedNumberOfNodesInBlock2);

    check_two_block_properties(tMeshDesignVariables, TEST_CONTEXT("Nodal densities"));
}

TEST_F(TwoBlockMeshOnDisk, ElementDensitiesToMeshDesignVariables)
{
    const auto tMesh = Mesh{mMeshFilePath};
    auto tDensities = std::vector<double>(EntityCounts{tMesh}.numberOfElements(), 1.0);
    std::iota(tDensities.begin(), tDensities.end(), 1.0);
    const auto tMeshDesignVariables = element_densities_to_mesh_design_variables(tDensities, tMesh);

    ASSERT_EQ(tMeshDesignVariables.mBlockDensities.size(), mExpectedNumberOfBlocks);
    EXPECT_EQ(tMeshDesignVariables.mBlockDensities.cbegin()->second.size(), mExpectedNumberOfElementsInBlock1);
    EXPECT_EQ(std::next(tMeshDesignVariables.mBlockDensities.cbegin())->second.size(),
              mExpectedNumberOfElementsInBlock2);

    check_two_block_properties(tMeshDesignVariables, TEST_CONTEXT("Element densities"));
}

}  // namespace plato::mesh::unittest
