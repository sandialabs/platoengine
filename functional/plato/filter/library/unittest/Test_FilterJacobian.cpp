#include <gtest/gtest.h>

#include <numeric>

#include "plato/filter/library/FilterInterface.hpp"
#include "plato/filter/library/FilterJacobian.hpp"
#include "plato/filter/library/unittest/TestFilter.hpp"

namespace plato::filter::library::unittest
{
namespace
{
const auto kBlock1 = std::vector<analysis::ScalarFieldValue>{{0, 0, 0.0}, {1, 1, 1.0}};
const auto kBlock2 = std::vector<analysis::ScalarFieldValue>{{0, 0, 0.0}, {1, 1, 1.0}, {2, 2, 2.0}};
const auto kTestMesh = analysis::AnalysisDomainMesh{"fake-file-name", {{0U, kBlock1}, {1U, kBlock2}}};
}  // namespace

TEST(FilterJacobian, MakeFilterJacobian)
{
    const auto tTestVector = linear_algebra::DynamicVector{0.0, 1.0, 2.0, 3.0};
    const auto tExpectedSize = kBlock1.size() + kBlock2.size() + tTestVector.size();

    {
        const auto tFilterJacobian = make_filter_jacobian(std::make_shared<TestFilter>(), kTestMesh);
        const auto tResult = tTestVector * tFilterJacobian;
        constexpr auto tExpectedValue = 1.0;
        const auto tExpected = std::vector<double>(tExpectedSize, tExpectedValue);

        EXPECT_EQ(tExpected, tResult.stdVector());
    }
    {
        const auto tFilterAdjointJacobian = make_filter_adjoint_jacobian(std::make_shared<TestFilter>(), kTestMesh);
        const auto tResult = tTestVector * tFilterAdjointJacobian;
        constexpr auto tExpectedValue = 2.0;
        const auto tExpected = std::vector<double>(tExpectedSize, tExpectedValue);

        EXPECT_EQ(tExpected, tResult.stdVector());
    }
}

}  // namespace plato::filter::library::unittest
