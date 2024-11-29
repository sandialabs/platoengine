#include <gtest/gtest.h>

#include "plato/analysis/AnalysisDomainMeshOperators.hpp"
#include "plato/filter/library/FilterFactory.hpp"
#include "plato/filter/library/unittest/TestFilter.hpp"
#include "plato/utilities/Zip.hpp"

namespace plato::filter::library::unittest
{
namespace
{
const auto kBlock1 = std::vector<analysis::ScalarFieldValue>{{0, 0, 0.0}, {1, 1, 1.0}};
const auto kBlock2 = std::vector<analysis::ScalarFieldValue>{{0, 0, 0.0}, {2, 2, 2.0}};
const auto kBlock3 = std::vector<analysis::ScalarFieldValue>{{2, 2, 2.0}};
const auto kTestMesh = analysis::AnalysisDomainMesh{"fake-file-name", {{0U, kBlock1}, {1U, kBlock2}, {3U, kBlock3}}};
}  // namespace

TEST(FilterFactor, MakeFilterFunction)
{
    const auto tTestVector = linear_algebra::DynamicVector{0.0, 1.0};
    const auto tExpectedSize = kBlock1.size() + kBlock2.size() + kBlock3.size() + tTestVector.size();
    const auto tFilterFunction = make_filter_function(std::make_shared<TestFilter>());

    // Filter
    {
        const auto tResult = tFilterFunction.evaluate<core::evaluation::kFunction>(kTestMesh);
        ASSERT_EQ(tResult.mBlockScalarField.size(), kTestMesh.mBlockScalarField.size());
        for (const auto& [tResultBlockField, tExpectedBlockField] :
             utilities::Zip{tResult.mBlockScalarField, kTestMesh.mBlockScalarField})
        {
            EXPECT_EQ(tResultBlockField.first, tExpectedBlockField.first);
            EXPECT_EQ(tResultBlockField.second, tExpectedBlockField.second);
        }
    }
    // Jacobian
    {
        const auto tJacobianResult =
            tTestVector * tFilterFunction.evaluate<core::evaluation::kFirstDerivative>(kTestMesh);
        const auto tExpectedJacobian = std::vector<double>(tExpectedSize, 1.0);
        EXPECT_EQ(tJacobianResult.stdVector(), tExpectedJacobian);
    }
    // Adjoint Jacobian
    {
        const auto tJacobianResult =
            tTestVector *
            tFilterFunction.evaluate<core::evaluation::kFirstDerivative, core::MatrixOrdering::kAdjoint>(kTestMesh);
        const auto tExpectedJacobian = std::vector<double>(tExpectedSize, 2.0);
        EXPECT_EQ(tJacobianResult.stdVector(), tExpectedJacobian);
    }
}

}  // namespace plato::filter::library::unittest
