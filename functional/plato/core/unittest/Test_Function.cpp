#include <gtest/gtest.h>

#include "plato/core/Function.hpp"
#include "plato/core/FunctionHelpers.hpp"
#include "plato/core/unittest/TestHelpers.hpp"
#include "plato/test_utilities/TwoDTestTypes.hpp"

namespace plato::core::unittest
{
TEST(Function, Evaluate)
{
    using EvaluateInfo = FunctionInfo<double, evaluation::kFunction>;
    using ScalarFunction = Function<double, EvaluateInfo>;

    constexpr auto tArbitraryValue = 42.0;
    const auto tF = ScalarFunction{[](const double) { return tArbitraryValue; }};
    EXPECT_EQ(tF.evaluate<evaluation::kFunction>(0.0), tArbitraryValue);
}

TEST(Function, EvaluateGradient)
{
    using FirstDerivativeInfo = FunctionInfo<double, evaluation::kFirstDerivative>;
    using ScalarFunctionDerivative = Function<double, FirstDerivativeInfo>;

    constexpr auto tArbitraryValue = 42.0;
    const auto tF = ScalarFunctionDerivative{[](const double) { return tArbitraryValue; }};
    EXPECT_EQ(tF.evaluate<evaluation::kFirstDerivative>(0.0), tArbitraryValue);
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

    constexpr auto tQuadraticExponent = 2.0;
    const auto tFunction = ScalarFunctionWithFirstDerivative{[](const double aX) { return aX * aX; },
                                                             [](const double aX) { return tQuadraticExponent * aX; }};

    constexpr auto tX = 3.0;
    EXPECT_EQ(tFunction.evaluate<evaluation::kFunction>(tX), tX * tX);
    EXPECT_EQ(tFunction.evaluate<evaluation::kFirstDerivative>(tX), tQuadraticExponent * tX);

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
        const auto tExpectedF = pft::make_two_d_vector(0.0, 0.0);
        const auto tExpectedDF = pft::make_two_d_matrix(0.0, 0.0, 1.0, 1.0);
        EXPECT_EQ(tF.evaluate<evaluation::kFunction>(tX), tExpectedF);
        EXPECT_EQ(tF.evaluate<evaluation::kFirstDerivative>(tX), tExpectedDF);
        EXPECT_EQ((tF.evaluate<evaluation::kFirstDerivative, MatrixOrdering::kAdjoint>(tX)),
                  pft::transpose(tExpectedDF));
    }
    {
        const auto tX = pft::make_two_d_vector(2.0, 1.0);
        const auto tExpectedF = pft::make_two_d_vector(2.0, 3.0);
        const auto tExpectedDF = pft::make_two_d_matrix(1.0, 2.0, 1.0, 1.0);
        EXPECT_EQ(tF.evaluate<evaluation::kFunction>(tX), tExpectedF);
        EXPECT_EQ(tF.evaluate<evaluation::kFirstDerivative>(tX), tExpectedDF);
        EXPECT_EQ((tF.evaluate<evaluation::kFirstDerivative, MatrixOrdering::kAdjoint>(tX)),
                  pft::transpose(tExpectedDF));
    }
}

TEST(Function, ScalarUsingMakeFunction)
{
    constexpr auto tCubicExponent = 3.0;
    const auto tFunction = make_function_with_first_derivative(
        [](const double aX) { return aX * aX * aX; }, [](const double aX) { return tCubicExponent * aX * aX; });

    constexpr auto tArgument = 3.0;
    EXPECT_EQ(tFunction.evaluate<evaluation::kFunction>(tArgument), tArgument * tArgument * tArgument);
    EXPECT_EQ(tFunction.evaluate<evaluation::kFirstDerivative>(tArgument), tCubicExponent * tArgument * tArgument);

    using FunctionType = decltype(tFunction);
    static_assert(FunctionType::isImplemented<0, MatrixOrdering::kOriginal>());
    static_assert(FunctionType::isImplemented<1, MatrixOrdering::kOriginal>());
    static_assert(!FunctionType::isImplemented<1, MatrixOrdering::kAdjoint>());
}

TEST(Function, SpecialCtors)
{
    constexpr auto tQuarticExponent = 4.0;
    auto tFunction =
        make_function_with_first_derivative([](const double aX) { return aX * aX * aX * aX; },
                                            [](const double aX) { return tQuarticExponent * aX * aX * aX; });
    // Copy
    {
        const auto tFunctionCopy = tFunction;  // NOLINT

        constexpr auto tArgument = 2.0;
        EXPECT_EQ(tFunctionCopy.evaluate<evaluation::kFunction>(tArgument),
                  tArgument * tArgument * tArgument * tArgument);
        EXPECT_EQ(tFunctionCopy.evaluate<evaluation::kFirstDerivative>(tArgument),
                  tQuarticExponent * tArgument * tArgument * tArgument);
    }
    // Move
    {
        const auto tFunctionMove = std::move(tFunction);  // NOLINT

        constexpr auto tArgument = (-2.0);
        EXPECT_EQ(tFunctionMove.evaluate<evaluation::kFunction>(tArgument),
                  tArgument * tArgument * tArgument * tArgument);
        EXPECT_EQ(tFunctionMove.evaluate<evaluation::kFirstDerivative>(tArgument),
                  tQuarticExponent * tArgument * tArgument * tArgument);
    }
}

TEST(Function, CompatibleFunction)
{
    using EvaluateInfo = FunctionInfo<double, evaluation::kFunction>;
    using FirstDerivativeInfo = FunctionInfo<double, evaluation::kFirstDerivative>;
    using SecondDerivativeInfo = FunctionInfo<double, evaluation::kSecondDerivative>;

    using FunctionWithFirstAndSecondDerivatives =
        Function<double, EvaluateInfo, FirstDerivativeInfo, SecondDerivativeInfo>;

    constexpr auto tArbitraryValue = 2.0;
    auto tFunctionWithFirstAndSecondDerivative =
        FunctionWithFirstAndSecondDerivatives{[](const double) { return 0.0; }, [](const double) { return 1.0; },
                                              [](const double) { return tArbitraryValue; }};
    // Copy construction
    {
        using FunctionWithFirstDerivative = Function<double, EvaluateInfo, FirstDerivativeInfo>;
        const auto tFunctionWithFirstDerivative =
            tFunctionWithFirstAndSecondDerivative.compatibleFunction<FunctionWithFirstDerivative>();

        EXPECT_EQ(tFunctionWithFirstDerivative.evaluate<evaluation::kFunction>(0.0), 0.0);
        EXPECT_EQ(tFunctionWithFirstDerivative.evaluate<evaluation::kFirstDerivative>(0.0), 1.0);
        EXPECT_FALSE(
            (tFunctionWithFirstDerivative.isImplemented<evaluation::kSecondDerivative, MatrixOrdering::kOriginal>()));
    }
    // Move construction
    {
        using FunctionWithNoDerivatives = Function<double, EvaluateInfo>;
        const auto tFunctionWithNoDerivatives =
            std::move(tFunctionWithFirstAndSecondDerivative).compatibleFunction<FunctionWithNoDerivatives>();

        EXPECT_EQ(tFunctionWithNoDerivatives.evaluate<evaluation::kFunction>(0.0), 0.0);
    }
}

}  // namespace plato::core::unittest
