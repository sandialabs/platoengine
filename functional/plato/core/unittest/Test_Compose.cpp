#include <gtest/gtest.h>

#include <cmath>

#include "plato/core/Compose.hpp"
#include "plato/core/Function.hpp"
#include "plato/test_utilities/TwoDTestTypes.hpp"

namespace plato::core::unittest
{
TEST(Composer, EvaluateComposition)
{
    // Tests f(g(tX)) and g(f(x)), with f(x) = x^2 and g(x) = 2x
    const auto tF = make_function([](const double aX) { return aX * aX; }, [](const double aX) { return 2.0 * aX; });
    const auto tG = make_function([](const double aX) { return 2.0 * aX; }, [](const double) { return 2.0; });
    const auto tFOfG = compose(tF, tG);
    EXPECT_EQ(tFOfG.f(1.0), 4.0);
    EXPECT_EQ(tFOfG.f(2.0), 16.0);

    const auto tGOfF = compose(tG, tF);
    EXPECT_EQ(tGOfF.f(1.0), 2.0);
    EXPECT_EQ(tGOfF.f(2.0), 8.0);
}

TEST(Composer, EvaluateCompositionGradient)
{
    // Tests derivative of f(g(tX)) and g(f(x)), with f(x) = x^2 and g(x) = 2x
    const auto tF = make_function([](const double aX) { return aX * aX; }, [](const double aX) { return 2.0 * aX; });
    const auto tG = make_function([](const double aX) { return 2.0 * aX; }, [](const double) { return 2.0; });
    const auto tFOfG = compose(tF, tG);
    EXPECT_EQ(tFOfG.df(1.0), 8.0);
    EXPECT_EQ(tFOfG.df(2.0), 16.0);

    const auto tGOfF = compose(tG, tF);
    EXPECT_EQ(tGOfF.df(1.0), 4.0);
    EXPECT_EQ(tGOfF.df(2.0), 8.0);
}

TEST(Composer, EvaluateCompositionThreeFunctions)
{
    // Tests h(g(f(tX))) and its derivative, with
    //  f(tX) = sqrt(x)
    //  g(tX) = sin(x)
    //  h(tX) = x^2
    const auto tF = make_function([](const double aX) { return std::sqrt(aX); },
                                  [](const double aX) { return 0.5 / std::sqrt(aX); });
    const auto tG =
        make_function([](const double aX) { return std::sin(aX); }, [](const double aX) { return std::cos(aX); });
    const auto tH = make_function([](const double aX) { return aX * aX; }, [](const double aX) { return 2.0 * aX; });

    const auto tHOfGOfF = compose(tH, compose(tG, tF));
    constexpr double tExpectedValue = 7.080734182735712e-01;  // From Matlab
    EXPECT_DOUBLE_EQ(tHOfGOfF.f(1.0), tExpectedValue);
    constexpr double tExpectedDerivative = 4.546487134128409e-01;  // From Matlab
    EXPECT_DOUBLE_EQ(tHOfGOfF.df(1.0), tExpectedDerivative);
}

TEST(Composer, TwoDFunctions)
{
    // Tests composition of a vector function and scalar function:
    // f(G(tX)), with X in R^2, G: R^2 -> R^2, f: R^2 -> R
    namespace pft = plato::test_utilities;

    const auto tF = make_function(pft::TwoDScalarFunction{}, pft::TwoDScalarFunctionGradient{});
    const auto tG = make_function(pft::TwoDVectorFunction{}, pft::TwoDVectorFunctionJacobian{});
    const auto tH = compose(tF, tG);
    {
        const auto tX = pft::TwoDVector{0.0, 0.0};
        const auto tExpectedF = 0.0;
        const auto tExpectedDF = pft::makeTwoDVector(0.0, 0.0);
        EXPECT_EQ(tH.f(tX), tExpectedF);
        EXPECT_EQ(tH.df(tX), tExpectedDF);
    }
    {
        const auto tX = pft::TwoDVector{1.0, -1.0};
        const auto tExpectedF = 1.0;
        const auto tExpectedDF = pft::makeTwoDVector(2.0, -2.0);
        EXPECT_EQ(tH.f(tX), tExpectedF);
        EXPECT_EQ(tH.df(tX), tExpectedDF);
    }
    {
        const auto tX = pft::TwoDVector{2.0, 4.0};
        const auto tExpectedF = 280.0;
        const auto tExpectedDF = pft::makeTwoDVector(172.0, 140.0);
        EXPECT_EQ(tH.f(tX), tExpectedF);
        EXPECT_EQ(tH.df(tX), tExpectedDF);
    }
}
}  // namespace plato::core::unittest
