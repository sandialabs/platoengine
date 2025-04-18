#include <gtest/gtest.h>

#include <string_view>

#include "plato/analysis/AnalysisDomainMesh.hpp"
#include "plato/analysis/AnalysisDomainMeshSequentialView.hpp"
#include "plato/filter/extension/IdentityFilter.hpp"
#include "plato/filter/library/FilterFactory.hpp"
#include "plato/filter/library/FilterJacobian.hpp"
#include "plato/linear_algebra/DynamicVector.hpp"
#include "plato/mesh/DesignVariableConversion.hpp"
#include "plato/test_utilities/TestContext.hpp"
#include "plato/utilities/Exception.hpp"
#include "plato/utilities/Zip.hpp"

namespace plato::filter::extension::unittest
{
namespace
{
constexpr std::string_view kMeshName = "the-mesh-is-a-lie.exo";
const auto kRho = std::vector{-1.0, 0.0, 1.0};
const auto kIDs = std::vector<std::size_t>{0, 1, 2};
const auto kMeshArgument = analysis::AnalysisDomainMesh{
    kMeshName,
    analysis::AnalysisDomainMesh::BlockScalarField{{1, analysis::combine_scalar_field_values_and_ids(kRho, kIDs)}}};
const auto kV = linear_algebra::DynamicVector<double>{-2.0, -1.0, 42.0};

void test_filtered_results(const analysis::AnalysisDomainMesh& aAnalysisDomainMesh,
                           const test_utilities::TestContext& aTestContext)
{
    const auto tMeshView = analysis::AnalysisDomainMeshSequentialView{aAnalysisDomainMesh};
    for (const auto& [tComputed, tExpected] : utilities::Zip{tMeshView, kRho})
    {
        EXPECT_EQ(static_cast<analysis::ScalarFieldValue>(tComputed).mValue, tExpected) << aTestContext;
    }
}
}  // namespace

TEST(IdentityFilter, Filter)
{
    const auto tAnalysisDomainMeshResult = IdentityFilter{}.filter(kMeshArgument);
    test_filtered_results(tAnalysisDomainMeshResult, TEST_CONTEXT("Direct filter"));
}

TEST(IdentityFilter, JacobianTimesVector)
{
    const auto tV = linear_algebra::DynamicVector<double>{-2.0, -1.0, 42.0};
    const linear_algebra::DynamicVector<double> tResult = IdentityFilter{}.rowVectorTimesJacobian(kMeshArgument, tV);
    EXPECT_EQ(tResult.stdVector(), tV.stdVector());
}

TEST(IdentityFilter, AdjointJacobianTimesVector)
{
    const auto tV = linear_algebra::DynamicVector<double>{-2.0, -1.0, 42.0};
    const linear_algebra::DynamicVector<double> tResult =
        IdentityFilter{}.rowVectorTimesAdjointJacobian(kMeshArgument, tV);
    EXPECT_EQ(tResult.stdVector(), tV.stdVector());
}

TEST(IdentityFilter, JacobianMultiplication)
{
    const auto tFilterJacobian = library::make_filter_jacobian(std::make_unique<IdentityFilter>(), kMeshArgument);
    const linear_algebra::DynamicVector<double> tResult = kV * tFilterJacobian;
    EXPECT_EQ(tResult.stdVector(), kV.stdVector());
}

TEST(IdentityFilter, JacobianBadDimensions)
{
    namespace pfu = plato::utilities;

    const auto tFilterJacobian = library::make_filter_jacobian(std::make_unique<IdentityFilter>(), kMeshArgument);

    const auto tVBad = linear_algebra::DynamicVector<double>{-2.0, -1.0, 42.0, 84.0};
    EXPECT_THROW(auto tTemp = IdentityFilter{}.rowVectorTimesJacobian(kMeshArgument, tVBad), pfu::Exception);
    EXPECT_THROW(auto tTemp2 = tVBad * tFilterJacobian, pfu::Exception);
}

TEST(IdentityFilter, Function)
{
    const auto tFilterFunction = make_identity_filter_function();
    const auto tAnalysisDomainMeshResult = tFilterFunction.evaluate<core::evaluation::kFunction>(kMeshArgument);
    test_filtered_results(tAnalysisDomainMeshResult, TEST_CONTEXT("Filter via filter function"));

    const linear_algebra::DynamicVector<double> tResult =
        kV * tFilterFunction.evaluate<core::evaluation::kFirstDerivative>(kMeshArgument);
    EXPECT_EQ(tResult.stdVector(), kV.stdVector());
}

TEST(IdentityFilter, Registration) { EXPECT_TRUE(library::is_new_filter_function_registered("identity_filter")); }

}  // namespace plato::filter::extension::unittest
