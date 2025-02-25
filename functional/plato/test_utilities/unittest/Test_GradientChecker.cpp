#include <gtest/gtest.h>

#include <cmath>
#include <valarray>

#include "plato/test_utilities/GradientChecker.hpp"

namespace plato::test_utilities::unittest
{
namespace
{
constexpr auto kTolerance = 1e-7;
}

TEST(GradientCheckerTests, SinCos)
{
    const auto tChecker = GradientChecker{[](double x) { return sin(x); }, [](double x) { return cos(x); }};
    constexpr auto tX = 1.0;
    constexpr auto tDirection = 1.0;

    EXPECT_NEAR(tChecker.maxFirstOrderTruncationError(tX, tDirection), 0.0, 1e-1);

    const auto tErrors = tChecker.finiteDifferenceErrors(tX, tDirection);
    ASSERT_FALSE(tErrors.empty());
    EXPECT_NEAR(tErrors.back().mValue, 0.0, kTolerance) << "Full gradient check:\n" << tChecker.table(tX, tDirection);
}

TEST(GradientCheckerTests, QuadraticVectorFunction)
{
    using Vector = std::valarray<double>;

    const auto tChecker = GradientChecker{[](const Vector& aV)
                                          {
                                              const auto tX = aV[0];
                                              const auto tY = aV[1];
                                              return tX * tX + tX * tY + tY * tY;
                                          },
                                          [](const Vector& tV)
                                          {
                                              const auto tX = tV[0];
                                              const auto tY = tV[1];
                                              return Vector{(2.0 * tX + tY), (2.0 * tY + tX)};
                                          }};
    const auto tV = Vector{2.0, 4.0};
    const auto tParameters = GradientCheckParameters{/*.mStepDelta=*/0.1,
                                                     /*.mNumSteps=*/7,
                                                     /*.mInitialStepSize=*/1.0};
    const auto tDirection = Vector{-1.0, 2.0};

    EXPECT_NEAR(tChecker.maxFirstOrderTruncationError(tV, tDirection, tParameters), 0.0, 1e-1);

    const auto tErrors = tChecker.finiteDifferenceErrors(tV, tDirection);
    ASSERT_FALSE(tErrors.empty());
    EXPECT_NEAR(tErrors.back().mValue, 0.0, kTolerance) << "Full gradient check:\n"
                                                        << tChecker.table(tV, tDirection, tParameters);
}

}  // namespace plato::test_utilities::unittest
