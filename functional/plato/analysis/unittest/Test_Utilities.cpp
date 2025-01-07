#include <gtest/gtest.h>

#include "plato/analysis/AnalysisDomainMesh.hpp"
#include "plato/analysis/AnalysisDomainMeshOperators.hpp"
#include "plato/analysis/Utilities.hpp"
#include "plato/test_utilities/TestContext.hpp"
#include "plato/utilities/Zip.hpp"

namespace plato::analysis
{
namespace
{
const auto kBlock1 = AnalysisDomainMesh::ScalarFieldVector{{0, 0, 0.0}, {1, 1, 1.0}, {2, 2, 2.0}};
const auto kBlock2 = AnalysisDomainMesh::ScalarFieldVector{{2, 2, 2.0}, {3, 3, 3.0}};
constexpr auto kBlock1ID = 1U;
constexpr auto kBlock2ID = 2U;
}  // namespace

TEST(Utilities, ZeroMeshField)
{
    auto tAnalysisDomainMesh = AnalysisDomainMesh{
        "fake-mesh", AnalysisDomainMesh::BlockScalarField{{kBlock1ID, kBlock1}, {kBlock2ID, kBlock2}}};
    tAnalysisDomainMesh = zero_scalar_field(std::move(tAnalysisDomainMesh));

    const auto tCheckResults = [](const AnalysisDomainMesh::ScalarFieldVector& aResult,
                                  const AnalysisDomainMesh::ScalarFieldVector& aExpected,
                                  const test_utilities::TestContext& aTestContext)
    {
        EXPECT_EQ(aResult.size(), aExpected.size()) << aTestContext;
        for (const auto& [tResult, tOriginal] : utilities::Zip{aResult, aExpected})
        {
            EXPECT_EQ(tResult.mGlobalMeshEntityID, tOriginal.mGlobalMeshEntityID) << aTestContext;
            EXPECT_EQ(tResult.mDesignVariableVectorIndex, tOriginal.mDesignVariableVectorIndex) << aTestContext;
            EXPECT_EQ(tResult.mValue, 0.0) << aTestContext;
        }
    };

    tCheckResults(tAnalysisDomainMesh.mBlockScalarField.at(kBlock1ID), kBlock1, TEST_CONTEXT("Block 1"));
    tCheckResults(tAnalysisDomainMesh.mBlockScalarField.at(kBlock2ID), kBlock2, TEST_CONTEXT("Block 2"));
}

TEST(Utilities, RemoveBlockFieldsOneBlock)
{
    auto tAnalysisDomainMesh = AnalysisDomainMesh{
        "fake-mesh", AnalysisDomainMesh::BlockScalarField{{kBlock1ID, kBlock1}, {kBlock2ID, kBlock2}}};
    tAnalysisDomainMesh = remove_block_fields(std::move(tAnalysisDomainMesh), {kBlock1ID});

    EXPECT_EQ(tAnalysisDomainMesh.mBlockScalarField.count(kBlock1ID), 0U);

    ASSERT_EQ(tAnalysisDomainMesh.mBlockScalarField.count(kBlock2ID), 1U);
    const auto tExpectedFieldWithRenumberedIndices = std::vector<analysis::ScalarFieldValue>{{2, 0, 2.0}, {3, 1, 3.0}};
    EXPECT_EQ(tAnalysisDomainMesh.mBlockScalarField.at(kBlock2ID), tExpectedFieldWithRenumberedIndices);
}

TEST(Utilities, RemoveBlockFieldsNoBlocks)
{
    auto tAnalysisDomainMesh = AnalysisDomainMesh{
        "fake-mesh", AnalysisDomainMesh::BlockScalarField{{kBlock1ID, kBlock1}, {kBlock2ID, kBlock2}}};
    tAnalysisDomainMesh = remove_block_fields(std::move(tAnalysisDomainMesh), {42U});

    EXPECT_EQ(tAnalysisDomainMesh.mBlockScalarField.count(kBlock1ID), 1U);
    EXPECT_EQ(tAnalysisDomainMesh.mBlockScalarField.at(kBlock1ID), kBlock1);
    EXPECT_EQ(tAnalysisDomainMesh.mBlockScalarField.count(kBlock2ID), 1U);
    EXPECT_EQ(tAnalysisDomainMesh.mBlockScalarField.at(kBlock2ID), kBlock2);
}

TEST(Utilities, RemoveBlockFieldsAllBlocks)
{
    auto tAnalysisDomainMesh = AnalysisDomainMesh{
        "fake-mesh", AnalysisDomainMesh::BlockScalarField{{kBlock1ID, kBlock1}, {kBlock2ID, kBlock2}}};
    tAnalysisDomainMesh = remove_block_fields(std::move(tAnalysisDomainMesh), {kBlock1ID, kBlock2ID});

    EXPECT_EQ(tAnalysisDomainMesh.mBlockScalarField.count(kBlock1ID), 0U);
    EXPECT_EQ(tAnalysisDomainMesh.mBlockScalarField.count(kBlock2ID), 0U);
}

}  // namespace plato::analysis
