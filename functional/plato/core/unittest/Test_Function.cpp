#include <gtest/gtest.h>

#include "plato/core/Function.hpp"
#include "plato/test_utilities/TwoDTestTypes.hpp"

namespace plato::core::unittest
{
TEST(PlatoFunctional, Evaluate)
{
    const auto tF =
        Function<double, double, double>{[](const double) { return 42.0; }, [](const double) { return 84.0; }};
    EXPECT_EQ(tF.f(0.0), 42.0);
}

TEST(PlatoFunctional, EvaluateGradient)
{
    const auto tF =
        Function<double, double, double>{[](const double) { return 42.0; }, [](const double) { return 84.0; }};
    EXPECT_EQ(tF.f(0.0), 42.0);
    EXPECT_EQ(tF.df(0.0), 84.0);
}

TEST(PlatoFunctional, MakeFunction)
{
    const auto tF = make_function([](const double aX) { return aX; }, [](const double aX) { return aX * aX; });
    EXPECT_EQ(tF.f(0.0), 0.0);
    EXPECT_EQ(tF.f(42.0), 42.0);
    EXPECT_EQ(tF.df(2.0), 4.0);
}

TEST(PlatoFunctional, TwoD)
{
    namespace pft = plato::test_utilities;
    const auto tF = make_function(pft::TwoDVectorFunction{}, pft::TwoDVectorFunctionJacobian{});

    {
        const auto tX = pft::TwoDVector{0.0, 0.0};
        const auto tExpectedF = pft::makeTwoDVector(0.0, 0.0);
        const auto tExpectedDF = pft::makeTwoDMatrix(0.0, 0.0, 1.0, 1.0);
        EXPECT_EQ(tF.f(tX), tExpectedF);
        EXPECT_EQ(tF.df(tX), tExpectedDF);
    }
    {
        const auto tX = pft::makeTwoDVector(2.0, 1.0);
        const auto tExpectedF = pft::makeTwoDVector(2.0, 3.0);
        const auto tExpectedDF = pft::makeTwoDMatrix(1.0, 2.0, 1.0, 1.0);
        EXPECT_EQ(tF.f(tX), tExpectedF);
        EXPECT_EQ(tF.df(tX), tExpectedDF);
    }
}

TEST(FunctionWithDerivatives, Scalar)
{
    using ScalarFInfo = FunctionInfo<double, evaluation::kFunction>;
    using ScalarFirstDerivativeInfo = FunctionInfo<double, evaluation::kFirstDerivative>;
    using ScalarFunctionWithFirstDerivative = FunctionWithDerivatives<double, ScalarFInfo, ScalarFirstDerivativeInfo>;

    const auto tFunction =
        ScalarFunctionWithFirstDerivative{[](const double x) { return x * x; }, [](const double x) { return 2.0 * x; }};

    EXPECT_EQ(tFunction.evaluate<evaluation::kFunction>(3.0), 3.0 * 3.0);
    EXPECT_EQ(tFunction.evaluate<evaluation::kFirstDerivative>(3.0), 2.0 * 3.0);

    static_assert(ScalarFunctionWithFirstDerivative::isImplemented<0, MatrixOrdering::kOriginal>());
    static_assert(ScalarFunctionWithFirstDerivative::isImplemented<1, MatrixOrdering::kOriginal>());
    static_assert(!ScalarFunctionWithFirstDerivative::isImplemented<2, MatrixOrdering::kOriginal>());
    static_assert(!ScalarFunctionWithFirstDerivative::isImplemented<0, MatrixOrdering::kAdjoint>());
    static_assert(!ScalarFunctionWithFirstDerivative::isImplemented<1, MatrixOrdering::kAdjoint>());
}

TEST(FunctionWithDerivatives, TwoD)
{
    namespace pft = plato::test_utilities;

    using VectorFInfo = FunctionInfo<pft::TwoDVector, evaluation::kFunction>;
    using VectorFirstDerivativeInfo = FunctionInfo<pft::TwoDMatrix, evaluation::kFirstDerivative>;
    using VectorFirstDerivativeAdjointInfo =
        FunctionInfo<pft::TwoDMatrix, evaluation::kFirstDerivative, MatrixOrdering::kAdjoint>;

    using VectorFunction = FunctionWithDerivatives<pft::TwoDVector, VectorFInfo, VectorFirstDerivativeInfo,
                                                   VectorFirstDerivativeAdjointInfo>;

    static_assert(VectorFunction::isImplemented<evaluation::kFirstDerivative, MatrixOrdering::kAdjoint>());

    const auto tF = VectorFunction{pft::TwoDVectorFunction{}, pft::TwoDVectorFunctionJacobian{},
                                   pft::TwoDVectorFunctionAdjointJacobian{}};

    {
        const auto tX = pft::TwoDVector{0.0, 0.0};
        const auto tExpectedF = pft::makeTwoDVector(0.0, 0.0);
        const auto tExpectedDF = pft::makeTwoDMatrix(0.0, 0.0, 1.0, 1.0);
        EXPECT_EQ(tF.evaluate<evaluation::kFunction>(tX), tExpectedF);
        EXPECT_EQ(tF.evaluate<evaluation::kFirstDerivative>(tX), tExpectedDF);
        EXPECT_EQ((tF.evaluate<evaluation::kFirstDerivative, MatrixOrdering::kAdjoint>(tX)),
                  pft::transpose(tExpectedDF));
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

}  // namespace plato::core::unittest
