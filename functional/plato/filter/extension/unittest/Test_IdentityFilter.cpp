#include <gtest/gtest.h>

#include <string_view>

#include "plato/design_variables/MeshDesignVariables.hpp"
#include "plato/design_variables/MeshDesignVariablesSequentialView.hpp"
#include "plato/filter/extension/IdentityFilter.hpp"
#include "plato/filter/library/FilterFactory.hpp"
#include "plato/filter/library/FilterJacobian.hpp"
#include "plato/input_parser/InputBlocks.hpp"
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
const auto kMeshArgument = design_variables::MeshDesignVariables{
    kMeshName, design_variables::MeshDesignVariables::BlockScalarField{
                   {1, design_variables::combine_scalar_field_values_and_ids(kRho, kIDs)}}};
const auto kV = linear_algebra::DynamicVector<double>{-2.0, -1.0, 42.0};

void test_filtered_results(const design_variables::MeshDesignVariables& aMeshDesignVariables,
                           const test_utilities::TestContext& aTestContext)
{
    const auto tMeshView = design_variables::MeshDesignVariablesSequentialView{aMeshDesignVariables};
    for (const auto& [tComputed, tExpected] : utilities::Zip{tMeshView, kRho})
    {
        EXPECT_EQ(static_cast<design_variables::ScalarFieldValue>(tComputed).mValue, tExpected) << aTestContext;
    }
}
}  // namespace

TEST(IdentityFilter, Filter)
{
    const auto tMeshDesignVariablesResult = IdentityFilter{}.filter(kMeshArgument);
    test_filtered_results(tMeshDesignVariablesResult, TEST_CONTEXT("Direct filter"));
}

TEST(IdentityFilter, JacobianTimesVector)
{
    const auto tV = linear_algebra::DynamicVector<double>{-2.0, -1.0, 42.0};
    const linear_algebra::DynamicVector<double> tResult = IdentityFilter{}.jacobianTimesVector(kMeshArgument, tV);
    EXPECT_EQ(tResult.stdVector(), tV.stdVector());
}

TEST(IdentityFilter, JacobianMultiplication)
{
    const auto tFilterJacobian = library::FilterJacobian{/*.mFilter=*/std::make_unique<IdentityFilter>(),
                                                         /*.mMeshDesignVariables=*/kMeshArgument};
    const linear_algebra::DynamicVector<double> tResult = kV * tFilterJacobian;
    EXPECT_EQ(tResult.stdVector(), kV.stdVector());
}

TEST(IdentityFilter, JacobianBadDimensions)
{
    namespace pfu = plato::utilities;

    const auto tFilterJacobian = library::FilterJacobian{/*.mFilter=*/std::make_unique<IdentityFilter>(),
                                                         /*.mMeshDesignVariables=*/kMeshArgument};

    const auto tVBad = linear_algebra::DynamicVector<double>{-2.0, -1.0, 42.0, 84.0};
    EXPECT_THROW(auto tTemp = IdentityFilter{}.jacobianTimesVector(kMeshArgument, tVBad), pfu::Exception);
    EXPECT_THROW(auto tTemp2 = tVBad * tFilterJacobian, pfu::Exception);
}

TEST(IdentityFilter, Function)
{
    const auto tFilterFunction = make_identity_filter_function();
    const auto tMeshDesignVariablesResult = tFilterFunction.f(kMeshArgument);
    test_filtered_results(tMeshDesignVariablesResult, TEST_CONTEXT("Filter via filter function"));

    const linear_algebra::DynamicVector<double> tResult = kV * tFilterFunction.df(kMeshArgument);
    EXPECT_EQ(tResult.stdVector(), kV.stdVector());
}
}  // namespace plato::filter::extension::unittest
