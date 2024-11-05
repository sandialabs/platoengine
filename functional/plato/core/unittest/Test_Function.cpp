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
    EXPECT_EQ(tFunction.evaluate<evaluation::kFirstDerivative>(tArgument), 3.0 * tArgument * tArgument);

    using FunctionType = decltype(tFunction);
    static_assert(FunctionType::isImplemented<0, MatrixOrdering::kOriginal>());
    static_assert(FunctionType::isImplemented<1, MatrixOrdering::kOriginal>());
    static_assert(!FunctionType::isImplemented<1, MatrixOrdering::kAdjoint>());
}

TEST(Function, SpecialCtors)
{
    auto tFunction = make_function_with_first_derivative([](const double x) { return x * x * x * x; },
                                                         [](const double x) { return 4.0 * x * x * x; });
    // Copy
    {
        const auto tFunctionCopy = tFunction;  // NOLINT

        constexpr auto tArgument = double{2.0};
        EXPECT_EQ(tFunctionCopy.evaluate<evaluation::kFunction>(tArgument),
                  tArgument * tArgument * tArgument * tArgument);
        EXPECT_EQ(tFunctionCopy.evaluate<evaluation::kFirstDerivative>(tArgument),
                  4.0 * tArgument * tArgument * tArgument);
    }
    // Move
    {
        const auto tFunctionMove = std::move(tFunction);  // NOLINT

        constexpr auto tArgument = double{-2.0};
        EXPECT_EQ(tFunctionMove.evaluate<evaluation::kFunction>(tArgument),
                  tArgument * tArgument * tArgument * tArgument);
        EXPECT_EQ(tFunctionMove.evaluate<evaluation::kFirstDerivative>(tArgument),
                  4.0 * tArgument * tArgument * tArgument);
    }
}

TEST(Function, IsOnlyMember)
{
    static_assert(!core::detail::is_only_member<int>());
    static_assert(core::detail::is_only_member<int, int>());
    static_assert(core::detail::is_only_member<int, const int&>());
    static_assert(!core::detail::is_only_member<int, int, int>());
}

TEST(Function, IsCopyable)
{
    using EvaluateInfo = FunctionInfo<double, evaluation::kFunction>;
    using FirstDerivativeInfo = FunctionInfo<double, evaluation::kFirstDerivative>;
    using SecondDerivativeInfo = FunctionInfo<double, evaluation::kSecondDerivative>;
    using FirstDerivativeAdjointInfo = FunctionInfo<double, evaluation::kSecondDerivative, MatrixOrdering::kAdjoint>;

    using DomainType1 = double;
    using DomainType2 = int;
    // Copyable
    static_assert(
        core::detail::is_copyable<DomainType1, std::tuple<EvaluateInfo>, DomainType1, std::tuple<EvaluateInfo>>());
    static_assert(core::detail::is_copyable<DomainType1, std::tuple<EvaluateInfo, FirstDerivativeInfo>, DomainType1,
                                            std::tuple<EvaluateInfo, FirstDerivativeInfo>>());
    static_assert(core::detail::is_copyable<DomainType1, std::tuple<EvaluateInfo>, DomainType1,
                                            std::tuple<EvaluateInfo, FirstDerivativeInfo>>());
    static_assert(
        core::detail::is_copyable<DomainType1, std::tuple<EvaluateInfo, FirstDerivativeInfo>, DomainType1,
                                  std::tuple<EvaluateInfo, FirstDerivativeInfo, FirstDerivativeAdjointInfo>>());

    // Not copyable
    static_assert(
        !core::detail::is_copyable<DomainType1, std::tuple<EvaluateInfo>, DomainType2, std::tuple<EvaluateInfo>>());
    static_assert(!core::detail::is_copyable<DomainType1, std::tuple<SecondDerivativeInfo>, DomainType1,
                                             std::tuple<EvaluateInfo, FirstDerivativeInfo>>());
    static_assert(!core::detail::is_copyable<DomainType1, std::tuple<EvaluateInfo, SecondDerivativeInfo>, DomainType1,
                                             std::tuple<EvaluateInfo, FirstDerivativeInfo>>());
    static_assert(!core::detail::is_copyable<DomainType1, std::tuple<EvaluateInfo, SecondDerivativeInfo>, DomainType1,
                                             std::tuple<EvaluateInfo>>());
}

TEST(Function, ConstructionFromDifferentType)
{
    using EvaluateInfo = FunctionInfo<double, evaluation::kFunction>;
    using FirstDerivativeInfo = FunctionInfo<double, evaluation::kFirstDerivative>;
    using SecondDerivativeInfo = FunctionInfo<double, evaluation::kSecondDerivative>;

    using FunctionWithFirstAndSecondDerivatives =
        Function<double, EvaluateInfo, FirstDerivativeInfo, SecondDerivativeInfo>;

    auto tFunctionWithSecondDerivative = FunctionWithFirstAndSecondDerivatives{
        [](const double) { return 0.0; }, [](const double) { return 1.0; }, [](const double) { return 2.0; }};
    // Copy construction
    {
        using FunctionWithFirstDerivative = Function<double, EvaluateInfo, FirstDerivativeInfo>;
        const auto tFunctionWithFirstDerivative =
            tFunctionWithSecondDerivative.compatibleFunction<FunctionWithFirstDerivative>();

        EXPECT_EQ(tFunctionWithFirstDerivative.evaluate<evaluation::kFunction>(0.0), 0.0);
        EXPECT_EQ(tFunctionWithFirstDerivative.evaluate<evaluation::kFirstDerivative>(0.0), 1.0);
        EXPECT_FALSE(
            (tFunctionWithFirstDerivative.isImplemented<evaluation::kSecondDerivative, MatrixOrdering::kOriginal>()));
    }
    // Move construction
    {
        using FunctionWithNoDerivatives = Function<double, EvaluateInfo>;
        const auto tFunctionWithNoDerivatives =
            std::move(tFunctionWithSecondDerivative).compatibleFunction<FunctionWithNoDerivatives>();

        EXPECT_EQ(tFunctionWithNoDerivatives.evaluate<evaluation::kFunction>(0.0), 0.0);
        EXPECT_THROW(
            [[maybe_unused]] const auto tResult = tFunctionWithSecondDerivative.evaluate<evaluation::kFunction>(0.0),
            std::bad_function_call);
    }
}

}  // namespace plato::core::unittest
