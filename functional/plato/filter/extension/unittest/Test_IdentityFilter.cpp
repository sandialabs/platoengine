#include <gtest/gtest.h>

#include <string_view>

#include "plato/core/MeshProxy.hpp"
#include "plato/filter/extension/IdentityFilter.hpp"
#include "plato/filter/library/FilterFactory.hpp"
#include "plato/filter/library/FilterJacobian.hpp"
#include "plato/input_parser/InputBlocks.hpp"
#include "plato/input_parser/InputEnumTypes.hpp"
#include "plato/linear_algebra/DynamicVector.hpp"
#include "plato/utilities/Exception.hpp"

namespace plato::filter::extension::unittest
{
namespace
{
constexpr std::string_view kMeshName = "the-mesh-is-a-lie.exo";
const auto kRho = std::vector{-1.0, 0.0, 1.0};
const auto kMeshArgument = core::MeshProxy{kMeshName, kRho};
const auto kV = linear_algebra::DynamicVector<double>{-2.0, -1.0, 42.0};
}  // namespace

TEST(FilterFactory, ValidIdentityFilter)
{
    auto tDensityTopology = input_parser::density_topology{};
    tDensityTopology.filter_type = input_parser::FilterTypes::kIdentity;
    EXPECT_NO_THROW(auto tFunction = library::make_filter_function(tDensityTopology));
}

TEST(IdentityFilter, Filter) { EXPECT_EQ(IdentityFilter{}.filter(kMeshArgument).mNodalDensities, kRho); }

TEST(IdentityFilter, JacobianTimesVector)
{
    const auto tV = linear_algebra::DynamicVector<double>{-2.0, -1.0, 42.0};
    const linear_algebra::DynamicVector<double> tResult = IdentityFilter{}.jacobianTimesVector(kMeshArgument, tV);
    EXPECT_EQ(tResult.stdVector(), tV.stdVector());
}

TEST(IdentityFilter, JacobianMultiplication)
{
    const auto tFilterJacobian = library::FilterJacobian{/*.mFilter=*/std::make_unique<IdentityFilter>(),
                                                         /*.mMeshProxy=*/kMeshArgument};
    const linear_algebra::DynamicVector<double> tResult = kV * tFilterJacobian;
    EXPECT_EQ(tResult.stdVector(), kV.stdVector());
}

TEST(IdentityFilter, JacobianBadDimensions)
{
    namespace pfu = plato::utilities;

    const auto tFilterJacobian = library::FilterJacobian{/*.mFilter=*/std::make_unique<IdentityFilter>(),
                                                         /*.mMeshProxy=*/kMeshArgument};

    const auto tVBad = linear_algebra::DynamicVector<double>{-2.0, -1.0, 42.0, 84.0};
    EXPECT_THROW(auto tTemp = IdentityFilter{}.jacobianTimesVector(kMeshArgument, tVBad), pfu::Exception);
    EXPECT_THROW(auto tTemp2 = tVBad * tFilterJacobian, pfu::Exception);
}

TEST(IdentityFilter, Function)
{
    const auto tFilterFunction = make_identity_filter_function();
    EXPECT_EQ(tFilterFunction.f(kMeshArgument).mNodalDensities, kRho);

    const linear_algebra::DynamicVector<double> tResult = kV * tFilterFunction.df(kMeshArgument);
    EXPECT_EQ(tResult.stdVector(), kV.stdVector());
}
}  // namespace plato::filter::extension::unittest
