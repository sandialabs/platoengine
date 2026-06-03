#include <gtest/gtest.h>

#include <cmath>
#include <valarray>

#include "plato/utilities/GradientChecker.hpp"

namespace plato::utilities::unittest
{
namespace
{
constexpr auto kTolerance = 1e-7;
}

TEST(GradientCheckerTests, SinCos)
{
    const auto tChecker = GradientChecker{[](double aX) { return sin(aX); }, [](double aX) { return cos(aX); }};
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

    const auto tChecker = GradientChecker{[](const Vector& aVector)
                                          {
                                              const auto tXCoordinate = aVector[0];
                                              const auto tYCoordinate = aVector[1];
                                              return tXCoordinate * tXCoordinate + tXCoordinate * tYCoordinate +
                                                     tYCoordinate * tYCoordinate;
                                          },
                                          [](const Vector& aVector)
                                          {
                                              const auto tXCoordinate = aVector[0];
                                              const auto tYCoordinate = aVector[1];
                                              constexpr auto tQuadraticPower = 2.0;
                                              return Vector{(tQuadraticPower * tXCoordinate + tYCoordinate),
                                                            (tQuadraticPower * tYCoordinate + tXCoordinate)};
                                          }};
    const auto tVector = Vector{2.0, 4.0};
    constexpr auto tParameters = GradientCheckParameters{.mStepDelta = 0.1, .mNumSteps = 7, .mInitialStepSize = 1.0};
    const auto tDirection = Vector{-1.0, 2.0};

    EXPECT_NEAR(tChecker.maxFirstOrderTruncationError(tVector, tDirection, tParameters), 0.0, 1e-1);

    const auto tErrors = tChecker.finiteDifferenceErrors(tVector, tDirection);
    ASSERT_FALSE(tErrors.empty());
    EXPECT_NEAR(tErrors.back().mValue, 0.0, kTolerance) << "Full gradient check:\n"
                                                        << tChecker.table(tVector, tDirection, tParameters);
}

}  // namespace plato::utilities::unittest
