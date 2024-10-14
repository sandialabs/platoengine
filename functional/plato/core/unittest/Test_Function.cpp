#include <gtest/gtest.h>

#include "plato/core/Function.hpp"
#include "plato/core/unittest/TestHelpers.hpp"
#include "plato/test_utilities/TwoDTestTypes.hpp"

namespace plato::core::unittest
{
TEST(Function, Evaluate)
{
    using EvaluateInfo = FunctionInfo<double, evaluation::kFunction>;
    using ScalarFunction = Function<double, EvaluateInfo>;

    const auto tF = ScalarFunction{[](const double) { return 42.0; }};
    EXPECT_EQ(tF.evaluate<evaluation::kFunction>(0.0), 42.0);
}

TEST(Function, EvaluateGradient)
{
    using FirstDerivativeInfo = FunctionInfo<double, evaluation::kFirstDerivative>;
    using ScalarFunctionDerivative = Function<double, FirstDerivativeInfo>;

    const auto tF = ScalarFunctionDerivative{[](const double) { return 84.0; }};
    EXPECT_EQ(tF.evaluate<evaluation::kFirstDerivative>(0.0), 84.0);
}

TEST(Function, MakeFunction)
{
    const auto tF = make_function_with_first_derivative([](const double aX) { return aX; },
                                                        [](const double aX) { return aX * aX; });
    EXPECT_EQ(tF.evaluate<evaluation::kFunction>(0.0), 0.0);
    EXPECT_EQ(tF.evaluate<evaluation::kFunction>(42.0), 42.0);
    EXPECT_EQ(tF.evaluate<evaluation::kFirstDerivative>(2.0), 4.0);
}

TEST(Function, Scalar)
{
    using ScalarFInfo = FunctionInfo<double, evaluation::kFunction>;
    using ScalarFirstDerivativeInfo = FunctionInfo<double, evaluation::kFirstDerivative>;
    using ScalarFunctionWithFirstDerivative = Function<double, ScalarFInfo, ScalarFirstDerivativeInfo>;

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

TEST(Function, TwoD)
{
    namespace pft = plato::test_utilities;

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
        EXPECT_EQ(tF.evaluate<evaluation::kFunction>(tX), tExpectedF);
        EXPECT_EQ(tF.evaluate<evaluation::kFirstDerivative>(tX), tExpectedDF);
        EXPECT_EQ((tF.evaluate<evaluation::kFirstDerivative, MatrixOrdering::kAdjoint>(tX)),
                  pft::transpose(tExpectedDF));
    }
}

TEST(Function, ScalarUsingMakeFunction)
{
    const auto tFunction = make_function_with_first_derivative([](const double x) { return x * x * x; },
                                                               [](const double x) { return 3.0 * x * x; });

    constexpr auto tArgument = double{3.0};
    EXPECT_EQ(tFunction.evaluate<evaluation::kFunction>(tArgument), tArgument * tArgument * tArgument);
    EXPECT_EQ(tFunction.evaluate<evaluation::kFirstDerivative>(3.0), 3.0 * tArgument * tArgument);

    using FunctionType = decltype(tFunction);
    static_assert(FunctionType::isImplemented<0, MatrixOrdering::kOriginal>());
    static_assert(FunctionType::isImplemented<1, MatrixOrdering::kOriginal>());
    static_assert(!FunctionType::isImplemented<1, MatrixOrdering::kAdjoint>());
}

}  // namespace plato::core::unittest
