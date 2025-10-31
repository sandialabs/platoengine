#include <gtest/gtest.h>

#include <cmath>

#include "plato/core/Compose.hpp"
#include "plato/core/Function.hpp"
#include "plato/core/unittest/TestHelpers.hpp"
#include "plato/test_utilities/TwoDTestTypes.hpp"

namespace plato::core::unittest
{

namespace
{
// Tests f(g(tX)) and g(f(x)), with f(x) = x^2 and g(x) = 2x
auto create_pair_of_compositions_of_quadratic_and_linear()
{
    const auto tF = make_function_with_first_derivative([](const double aX) { return aX * aX; },
                                                        [](const double aX) { return 2.0 * aX; });
    const auto tG =
        make_function_with_first_derivative([](const double aX) { return 2.0 * aX; }, [](const double) { return 2.0; });

    return std::make_pair(compose(tF, tG), compose(tG, tF));
}

}  // namespace

TEST(Composer, EvaluateComposition)
{
    const auto [tFOfG, tGOfF] = create_pair_of_compositions_of_quadratic_and_linear();
    EXPECT_EQ(tFOfG.evaluate<evaluation::kFunction>(1.0), 4.0);
    EXPECT_EQ(tFOfG.evaluate<evaluation::kFunction>(2.0), 16.0);

    EXPECT_EQ(tGOfF.evaluate<evaluation::kFunction>(1.0), 2.0);
    EXPECT_EQ(tGOfF.evaluate<evaluation::kFunction>(2.0), 8.0);
}

TEST(Composer, EvaluateCompositionGradient)
{
    const auto [tFOfG, tGOfF] = create_pair_of_compositions_of_quadratic_and_linear();
    EXPECT_EQ(tFOfG.evaluate<evaluation::kFirstDerivative>(1.0), 8.0);
    EXPECT_EQ(tFOfG.evaluate<evaluation::kFirstDerivative>(2.0), 16.0);

    EXPECT_EQ(tGOfF.evaluate<evaluation::kFirstDerivative>(1.0), 4.0);
    EXPECT_EQ(tGOfF.evaluate<evaluation::kFirstDerivative>(2.0), 8.0);
}

TEST(Composer, EvaluateCompositionThreeFunctions)
{
    // Tests h(g(f(tX))) and its derivative, with
    //  f(tX) = sqrt(x)
    //  g(tX) = sin(x)
    //  h(tX) = x^2
    const auto tF = make_function_with_first_derivative([](const double aX) { return std::sqrt(aX); },
                                                        [](const double aX) { return 0.5 / std::sqrt(aX); });
    const auto tG = make_function_with_first_derivative([](const double aX) { return std::sin(aX); },
                                                        [](const double aX) { return std::cos(aX); });
    const auto tH = make_function_with_first_derivative([](const double aX) { return aX * aX; },
                                                        [](const double aX) { return 2.0 * aX; });

    const auto tHOfGOfF = compose(tH, compose(tG, tF));
    constexpr double tExpectedValue = 7.080734182735712e-01;  // From Matlab
    EXPECT_DOUBLE_EQ(tHOfGOfF.evaluate<evaluation::kFunction>(1.0), tExpectedValue);
    constexpr double tExpectedDerivative = 4.546487134128409e-01;  // From Matlab
    EXPECT_DOUBLE_EQ(tHOfGOfF.evaluate<evaluation::kFirstDerivative>(1.0), tExpectedDerivative);
}

TEST(Composer, TwoDFunctions)
{
    // Tests composition of a vector function and scalar function:
    // f(G(tX)), with X in R^2, G: R^2 -> R^2, f: R^2 -> R
    namespace pft = plato::test_utilities;

    const auto tF = make_function_with_first_derivative(pft::TwoDScalarFunction{}, pft::TwoDScalarFunctionGradient{});
    const auto tG = make_function_with_first_derivative(pft::TwoDVectorFunction{}, pft::TwoDVectorFunctionJacobian{});
    const auto tH = compose(tF, tG);
    {
        const auto tX = pft::TwoDVector{0.0, 0.0};
        const auto tExpectedF = 0.0;
        const auto tExpectedDF = pft::makeTwoDVector(0.0, 0.0);
        EXPECT_EQ(tH.evaluate<evaluation::kFunction>(tX), tExpectedF);
        EXPECT_EQ(tH.evaluate<evaluation::kFirstDerivative>(tX), tExpectedDF);
    }
    {
        const auto tX = pft::TwoDVector{1.0, -1.0};
        const auto tExpectedF = 1.0;
        const auto tExpectedDF = pft::makeTwoDVector(2.0, -2.0);
        EXPECT_EQ(tH.evaluate<evaluation::kFunction>(tX), tExpectedF);
        EXPECT_EQ(tH.evaluate<evaluation::kFirstDerivative>(tX), tExpectedDF);
    }
    {
        const auto tX = pft::TwoDVector{2.0, 4.0};
        const auto tExpectedF = 280.0;
        const auto tExpectedDF = pft::makeTwoDVector(172.0, 140.0);
        EXPECT_EQ(tH.evaluate<evaluation::kFunction>(tX), tExpectedF);
        EXPECT_EQ(tH.evaluate<evaluation::kFirstDerivative>(tX), tExpectedDF);
    }
}

TEST(Composer, ScalarTransformation)
{
    // Tests composition of a scalar function with a scalar transformation:
    // f(g(tX)), with tX in R^2, g: R^2 -> R, f: R -> R
    namespace pft = plato::test_utilities;

    const auto tG = make_function_with_first_derivative(pft::TwoDScalarFunction{}, pft::TwoDScalarFunctionGradient{});
    const auto tF = make_function_with_first_derivative([](const double aArg) { return 1.0 / aArg; },
                                                        [](const double aArg) { return -1.0 / aArg / aArg; });
    const auto tH = compose(tF, tG);
    {
        const auto tX = pft::TwoDVector{1.0, 0.0};
        const auto tExpectedValue = 1.0;
        const auto tExpectedDerivative = pft::makeTwoDVector(-2.0, 0.0);
        EXPECT_EQ(tH.evaluate<evaluation::kFunction>(tX), tExpectedValue);
        EXPECT_EQ(tH.evaluate<evaluation::kFirstDerivative>(tX), tExpectedDerivative);
    }
    {
        const auto tX = pft::TwoDVector{1.0, 1.0};
        const auto tExpectedValue = 0.5;
        const auto tExpectedDerivative = pft::makeTwoDVector(-0.5, -3.0 / 4.0);
        EXPECT_EQ(tH.evaluate<evaluation::kFunction>(tX), tExpectedValue);
        EXPECT_EQ(tH.evaluate<evaluation::kFirstDerivative>(tX), tExpectedDerivative);
    }
    {
        const auto tX = pft::TwoDVector{-4.0, 3.0};
        const auto tExpectedValue = 1.0 / 43.0;
        const auto tExpectedDerivative = pft::makeTwoDVector(8.0 / 43.0 / 43.0, -27.0 / 43.0 / 43.0);
        EXPECT_EQ(tH.evaluate<evaluation::kFunction>(tX), tExpectedValue);
        EXPECT_EQ(tH.evaluate<evaluation::kFirstDerivative>(tX), tExpectedDerivative);
    }
}

TEST(Compose, CompositionScalarFunctions)
{
    using ScalarFInfo = FunctionInfo<double, evaluation::kFunction>;
    using ScalarFirstDerivativeInfo = FunctionInfo<double, evaluation::kFirstDerivative>;
    using ScalarFunctionWithFirstDerivative = Function<double, ScalarFInfo, ScalarFirstDerivativeInfo>;

    const auto tFunction1 =
        ScalarFunctionWithFirstDerivative{[](const double x) { return x + 1; }, [](const double) { return 1.0; }};

    const auto tFunction2 =
        ScalarFunctionWithFirstDerivative{[](const double x) { return x * x; }, [](const double x) { return 2.0 * x; }};

    {
        // x^2 + 1
        const auto tComposition = compose(tFunction1, tFunction2);
        EXPECT_EQ(tComposition.evaluate<evaluation::kFunction>(0.0), 1.0);
        EXPECT_EQ(tComposition.evaluate<evaluation::kFirstDerivative>(0.0), 0.0);
        EXPECT_EQ(tComposition.evaluate<evaluation::kFunction>(-1.0), 2.0);
        EXPECT_EQ(tComposition.evaluate<evaluation::kFirstDerivative>(-1.0), -2.0);
        EXPECT_EQ(tComposition.evaluate<evaluation::kFunction>(1.0), 2.0);
        EXPECT_EQ(tComposition.evaluate<evaluation::kFirstDerivative>(1.0), 2.0);
    }
    {
        // (x + 1)^2
        const auto tComposition = compose(tFunction2, tFunction1);
        EXPECT_EQ(tComposition.evaluate<evaluation::kFunction>(0.0), 1.0);
        EXPECT_EQ(tComposition.evaluate<evaluation::kFirstDerivative>(0.0), 2.0);
        EXPECT_EQ(tComposition.evaluate<evaluation::kFunction>(-1.0), 0.0);
        EXPECT_EQ(tComposition.evaluate<evaluation::kFirstDerivative>(-1.0), 0.0);
        EXPECT_EQ(tComposition.evaluate<evaluation::kFunction>(1.0), 4.0);
        EXPECT_EQ(tComposition.evaluate<evaluation::kFirstDerivative>(1.0), 4.0);
    }
}

TEST(Compose, CompositionVectorFunctions)
{
    namespace pft = plato::test_utilities;

    const auto tF = VectorFunction{pft::TwoDVectorFunction{}, pft::TwoDVectorFunctionJacobian{},
                                   pft::TwoDVectorFunctionAdjointJacobian{}};

    const auto tComposition = compose(tF, tF);

    {
        const auto tX = pft::TwoDVector{0.0, 0.0};
        const auto tExpectedF = pft::makeTwoDVector(0.0, 0.0);
        const auto tExpectedDF = pft::makeTwoDMatrix(0.0, 0.0, 1.0, 1.0);
        const auto tExpectedAdjointDF = pft::makeTwoDMatrix(0.0, 1.0, 0.0, 1.0);
        EXPECT_EQ(tComposition.evaluate<evaluation::kFunction>(tX), tExpectedF);
        EXPECT_EQ(tComposition.evaluate<evaluation::kFirstDerivative>(tX), tExpectedDF);
        EXPECT_EQ((tComposition.evaluate<evaluation::kFirstDerivative, MatrixOrdering::kAdjoint>(tX)),
                  tExpectedAdjointDF);
    }
    {
        const auto tX = pft::TwoDVector{1.0, 0.5};
        const auto tExpectedF = pft::makeTwoDVector(0.75, 2.0);
        const auto tExpectedDF = pft::makeTwoDMatrix(1.25, 2.0, 1.5, 2.0);
        const auto tExpectedAdjointDF = pft::makeTwoDMatrix(1.25, 1.5, 2.0, 2.0);
        EXPECT_EQ(tComposition.evaluate<evaluation::kFunction>(tX), tExpectedF);
        EXPECT_EQ(tComposition.evaluate<evaluation::kFirstDerivative>(tX), tExpectedDF);
        EXPECT_EQ((tComposition.evaluate<evaluation::kFirstDerivative, MatrixOrdering::kAdjoint>(tX)),
                  tExpectedAdjointDF);
    }
}

TEST(Compose, FunctionSizesRequirement)
{
    namespace pft = plato::test_utilities;
    using VectorFunctionNoAdjoint = Function<plato::test_utilities::TwoDVector, VectorFInfo, VectorFirstDerivativeInfo>;

    static_assert(plato::core::detail::kFunctionCompositionSizesRequirement<VectorFunctionNoAdjoint, VectorFunction>);
    static_assert(!plato::core::detail::kFunctionCompositionSizesRequirement<VectorFunction, VectorFunctionNoAdjoint>);
}

TEST(Compose, CompositionMismatchedFunctions)
{
    namespace pft = plato::test_utilities;
    using VectorFunctionNoAdjoint = Function<plato::test_utilities::TwoDVector, VectorFInfo, VectorFirstDerivativeInfo>;

    const auto tF = VectorFunctionNoAdjoint{pft::TwoDVectorFunction{}, pft::TwoDVectorFunctionJacobian{}};
    const auto tG = VectorFunction{pft::TwoDVectorFunction{}, pft::TwoDVectorFunctionJacobian{},
                                   pft::TwoDVectorFunctionAdjointJacobian{}};

    const auto tComposition = compose(tF, tG);
    using CompositionType = decltype(tComposition);
    static_assert(CompositionType::isImplemented<evaluation::kFunction, MatrixOrdering::kOriginal>());
    static_assert(CompositionType::isImplemented<evaluation::kFirstDerivative, MatrixOrdering::kOriginal>());
    static_assert(!CompositionType::isImplemented<evaluation::kFirstDerivative, MatrixOrdering::kAdjoint>());
}

}  // namespace plato::core::unittest
