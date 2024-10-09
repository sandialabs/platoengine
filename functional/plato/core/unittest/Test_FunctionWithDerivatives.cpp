#include <gtest/gtest.h>

#include "plato/core/Compose.hpp"
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

    EXPECT_EQ(tFunction.evaluate<0>(3.0), 3.0 * 3.0);
    EXPECT_EQ(tFunction.evaluate<1>(3.0), 2.0 * 3.0);

    static_assert(ScalarFunctionWithFirstDerivative::isImplemented<0, MatrixOrdering::kOriginal>());
    static_assert(ScalarFunctionWithFirstDerivative::isImplemented<1, MatrixOrdering::kOriginal>());
    static_assert(!ScalarFunctionWithFirstDerivative::isImplemented<2, MatrixOrdering::kOriginal>());
    static_assert(!ScalarFunctionWithFirstDerivative::isImplemented<0, MatrixOrdering::kAdjoint>());
    static_assert(!ScalarFunctionWithFirstDerivative::isImplemented<1, MatrixOrdering::kAdjoint>());
}

TEST(FunctionWithDerivatives, TwoD)
{
    namespace pft = plato::test_utilities;

    using VectorFInfo = FunctionInfo<pft::TwoDVector, 0>;
    using VectorFirstDerivativeInfo = FunctionInfo<pft::TwoDMatrix, 1>;
    using VectorFirstDerivativeAdjointInfo = FunctionInfo<pft::TwoDMatrix, 1, MatrixOrdering::kAdjoint>;

    using VectorFunction = FunctionWithDerivatives<pft::TwoDVector, VectorFInfo, VectorFirstDerivativeInfo,
                                                   VectorFirstDerivativeAdjointInfo>;

    static_assert(VectorFunction::isImplemented<evaluate::kFirstDerivative, MatrixOrdering::kAdjoint>());

    const auto tF = VectorFunction{pft::TwoDVectorFunction{}, pft::TwoDVectorFunctionJacobian{},
                                   pft::TwoDVectorFunctionAdjointJacobian{}};

    {
        const auto tX = pft::TwoDVector{0.0, 0.0};
        const auto tExpectedF = pft::makeTwoDVector(0.0, 0.0);
        const auto tExpectedDF = pft::makeTwoDMatrix(0.0, 0.0, 1.0, 1.0);
        EXPECT_EQ(tF.evaluate<evaluate::kFunction>(tX), tExpectedF);
        EXPECT_EQ(tF.evaluate<evaluate::kFirstDerivative>(tX), tExpectedDF);
        EXPECT_EQ((tF.evaluate<evaluate::kFirstDerivative, MatrixOrdering::kAdjoint>(tX)), pft::transpose(tExpectedDF));
    }
    {
        const auto tX = pft::makeTwoDVector(2.0, 1.0);
        const auto tExpectedF = pft::makeTwoDVector(2.0, 3.0);
        const auto tExpectedDF = pft::makeTwoDMatrix(1.0, 2.0, 1.0, 1.0);
        EXPECT_EQ(tF.evaluate<0>(tX), tExpectedF);
        EXPECT_EQ(tF.evaluate<1>(tX), tExpectedDF);
        EXPECT_EQ((tF.evaluate<1, MatrixOrdering::kAdjoint>(tX)), pft::transpose(tExpectedDF));
    }
}

TEST(FunctionWithDerivatives, CompositionScalarFunctions)
{
    using ScalarFInfo = FunctionInfo<double, 0>;
    using ScalarFirstDerivativeInfo = FunctionInfo<double, 1>;
    using ScalarFunctionWithFirstDerivative = FunctionWithDerivatives<double, ScalarFInfo, ScalarFirstDerivativeInfo>;

    const auto tFunction1 =
        ScalarFunctionWithFirstDerivative{[](const double x) { return x + 1; }, [](const double) { return 1.0; }};

    const auto tFunction2 =
        ScalarFunctionWithFirstDerivative{[](const double x) { return x * x; }, [](const double x) { return 2.0 * x; }};

    {
        // x^2 + 1
        const auto tComposition = compose_new(tFunction1, tFunction2);
        EXPECT_EQ(tComposition.evaluate<0>(0.0), 1.0);
        EXPECT_EQ(tComposition.evaluate<1>(0.0), 0.0);
        EXPECT_EQ(tComposition.evaluate<0>(-1.0), 2.0);
        EXPECT_EQ(tComposition.evaluate<1>(-1.0), -2.0);
        EXPECT_EQ(tComposition.evaluate<0>(1.0), 2.0);
        EXPECT_EQ(tComposition.evaluate<1>(1.0), 2.0);
    }
    {
        // (x + 1)^2
        const auto tComposition = compose_new(tFunction2, tFunction1);
        EXPECT_EQ(tComposition.evaluate<0>(0.0), 1.0);
        EXPECT_EQ(tComposition.evaluate<1>(0.0), 2.0);
        EXPECT_EQ(tComposition.evaluate<0>(-1.0), 0.0);
        EXPECT_EQ(tComposition.evaluate<1>(-1.0), 0.0);
        EXPECT_EQ(tComposition.evaluate<0>(1.0), 4.0);
        EXPECT_EQ(tComposition.evaluate<1>(1.0), 4.0);
    }
}

TEST(FunctionWithDerivatives, CompositionVectorFunctions)
{
    namespace pft = plato::test_utilities;

    using VectorFInfo = FunctionInfo<pft::TwoDVector, 0>;
    using VectorFirstDerivativeInfo = FunctionInfo<pft::TwoDMatrix, 1>;
    using VectorFirstDerivativeAdjointInfo = FunctionInfo<pft::TwoDMatrix, 1, MatrixOrdering::kAdjoint>;

    using VectorFunction = FunctionWithDerivatives<pft::TwoDVector, VectorFInfo, VectorFirstDerivativeInfo,
                                                   VectorFirstDerivativeAdjointInfo>;

    const auto tF = VectorFunction{pft::TwoDVectorFunction{}, pft::TwoDVectorFunctionJacobian{},
                                   pft::TwoDVectorFunctionAdjointJacobian{}};

    const auto tComposition = compose_new(tF, tF);

    {
        const auto tX = pft::TwoDVector{0.0, 0.0};
        const auto tExpectedF = pft::makeTwoDVector(0.0, 0.0);
        const auto tExpectedDF = pft::makeTwoDMatrix(0.0, 0.0, 1.0, 1.0);
        const auto tExpectedAdjointDF = pft::makeTwoDMatrix(0.0, 1.0, 0.0, 1.0);
        EXPECT_EQ(tComposition.evaluate<0>(tX), tExpectedF);
        EXPECT_EQ(tComposition.evaluate<1>(tX), tExpectedDF);
        EXPECT_EQ((tComposition.evaluate<1, MatrixOrdering::kAdjoint>(tX)), tExpectedAdjointDF);
    }
    {
        const auto tX = pft::TwoDVector{1.0, 0.5};
        const auto tExpectedF = pft::makeTwoDVector(0.75, 2.0);
        const auto tExpectedDF = pft::makeTwoDMatrix(1.25, 2.0, 1.5, 2.0);
        const auto tExpectedAdjointDF = pft::makeTwoDMatrix(1.25, 1.5, 2.0, 2.0);
        EXPECT_EQ(tComposition.evaluate<0>(tX), tExpectedF);
        EXPECT_EQ(tComposition.evaluate<1>(tX), tExpectedDF);
        EXPECT_EQ((tComposition.evaluate<1, MatrixOrdering::kAdjoint>(tX)), tExpectedAdjointDF);
    }
}

}  // namespace plato::core::unittest
