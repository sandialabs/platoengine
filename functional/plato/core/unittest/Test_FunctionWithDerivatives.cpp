#include <gtest/gtest.h>

#include "plato/core/FunctionWithDerivatives.hpp"
#include "plato/test_utilities/TwoDTestTypes.hpp"

namespace plato::core::unittest
{
TEST(FunctionWithDerivatives, Scalar)
{
    using ScalarFInfo = FunctionInfo<double, 0>;
    using ScalarFirstDerivativeInfo = FunctionInfo<double, 1>;
    using ScalarFunctionWithFirstDerivative = FunctionWithDerivatives<double, ScalarFInfo, ScalarFirstDerivativeInfo>;

    const auto tFunction =
        ScalarFunctionWithFirstDerivative{[](const double x) { return x * x; }, [](const double x) { return 2.0 * x; }};

    EXPECT_EQ(tFunction.f<0>(3.0), 3.0 * 3.0);
    EXPECT_EQ(tFunction.f<1>(3.0), 2.0 * 3.0);
}

TEST(FunctionWithDerivatives, TwoD)
{
    namespace pft = plato::test_utilities;

    using VectorFInfo = FunctionInfo<pft::TwoDVector, 0>;
    using VectorFirstDerivativeInfo = FunctionInfo<pft::TwoDMatrix, 1>;
    using VectorFirstDerivativeAdjointInfo = FunctionInfo<pft::TwoDMatrix, 1, MatrixOrdering::kAdjoint>;

    using VectorFunction = FunctionWithDerivatives<pft::TwoDVector, VectorFInfo, VectorFirstDerivativeInfo,
                                                   VectorFirstDerivativeAdjointInfo>;

    const auto tF = VectorFunction{pft::TwoDVectorFunction{}, pft::TwoDVectorFunctionJacobian{},
                                   pft::TwoDVectorFunctionAdjointJacobian{}};

    {
        const auto tX = pft::TwoDVector{0.0, 0.0};
        const auto tExpectedF = pft::makeTwoDVector(0.0, 0.0);
        const auto tExpectedDF = pft::makeTwoDMatrix(0.0, 0.0, 1.0, 1.0);
        EXPECT_EQ(tF.f<0>(tX), tExpectedF);
        EXPECT_EQ(tF.f<1>(tX), tExpectedDF);
        EXPECT_EQ((tF.f<1, MatrixOrdering::kAdjoint>(tX)), pft::transpose(tExpectedDF));
    }
    {
        const auto tX = pft::makeTwoDVector(2.0, 1.0);
        const auto tExpectedF = pft::makeTwoDVector(2.0, 3.0);
        const auto tExpectedDF = pft::makeTwoDMatrix(1.0, 2.0, 1.0, 1.0);
        EXPECT_EQ(tF.f<0>(tX), tExpectedF);
        EXPECT_EQ(tF.f<1>(tX), tExpectedDF);
        EXPECT_EQ((tF.f<1, MatrixOrdering::kAdjoint>(tX)), pft::transpose(tExpectedDF));
    }
}

}  // namespace plato::core::unittest
