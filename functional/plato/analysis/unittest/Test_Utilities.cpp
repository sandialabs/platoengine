#include <gtest/gtest.h>

#include "plato/analysis/AnalysisDomainMesh.hpp"
#include "plato/analysis/Utilities.hpp"
#include "plato/test_utilities/TestContext.hpp"
#include "plato/utilities/Zip.hpp"

namespace plato::analysis
{
TEST(Utilities, ZeroMeshField)
{
    const auto tBlock1 = AnalysisDomainMesh::ScalarFieldVector{{0, 0, 0.0}, {1, 1, 1.0}, {2, 2, 2.0}};
    const auto tBlock2 = AnalysisDomainMesh::ScalarFieldVector{{2, 2, 2.0}, {3, 3, 3.0}};
    constexpr auto tBlock1ID = 1U;
    constexpr auto tBlock2ID = 2U;
    auto tAnalysisDomainMesh = AnalysisDomainMesh{
        "fake-mesh", AnalysisDomainMesh::BlockScalarField{{tBlock1ID, tBlock1}, {tBlock2ID, tBlock2}}};
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

    tCheckResults(tAnalysisDomainMesh.mBlockScalarField.at(tBlock1ID), tBlock1, TEST_CONTEXT("Block 1"));
    tCheckResults(tAnalysisDomainMesh.mBlockScalarField.at(tBlock2ID), tBlock2, TEST_CONTEXT("Block 2"));
}
}  // namespace plato::analysis
