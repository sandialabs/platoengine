#include <gtest/gtest.h>

#include "plato/core/Function.hpp"
#include "plato/criteria/library/ObjectiveInputBlock.hpp"
#include "plato/criteria/library/ObjectiveReciprocal.hpp"
#include "plato/test_utilities/TwoDTestTypes.hpp"

namespace plato::criteria::library::unittest
{
TEST(ObjectiveReciprocal, ReciprocalFunction)
{
    const auto tFunction = reciprocal_function();
    constexpr double tArg = 77.0;

    EXPECT_EQ(tFunction.evaluate<core::evaluation::kFunction>(tArg), 1.0 / tArg);
    EXPECT_EQ(tFunction.evaluate<core::evaluation::kFirstDerivative>(tArg), -1.0 / tArg / tArg);
}

TEST(ObjectiveReciprocal, MakeReciprocalCriterionFunction)
{
    namespace pft = plato::test_utilities;
    const auto tF =
        core::make_function_with_first_derivative(pft::TwoDScalarFunction{}, pft::TwoDScalarFunctionGradient{});

    constexpr double tX1 = -4.0;
    constexpr double tX2 = 3.0;
    const auto tX = pft::TwoDVector{tX1, tX2};

    const auto tUnmodifiedValue = tX1 * tX1 + tX2 * tX2 * tX2;
    const auto tUnmodifiedDerivative = pft::makeTwoDVector(2.0 * tX1, 3.0 * tX2 * tX2);

    // returns original function with kMinimize objective goal
    {
        const auto tModifiedF = make_reciprocal_criterion_function(tF, ObjectiveGoal::kMinimize);
        EXPECT_EQ(tModifiedF.evaluate<core::evaluation::kFunction>(tX), tUnmodifiedValue);
        EXPECT_EQ(tModifiedF.evaluate<core::evaluation::kFirstDerivative>(tX), tUnmodifiedDerivative);
    }
    // returns original function with kMinimizeNegation objective goal
    {
        const auto tModifiedF = make_reciprocal_criterion_function(tF, ObjectiveGoal::kMinimizeNegation);
        EXPECT_EQ(tModifiedF.evaluate<core::evaluation::kFunction>(tX), tUnmodifiedValue);
        EXPECT_EQ(tModifiedF.evaluate<core::evaluation::kFirstDerivative>(tX), tUnmodifiedDerivative);
    }
    // returns reciprocated function with kMinimizeReciprocal objective goal
    {
        const auto tModifiedF = make_reciprocal_criterion_function(tF, ObjectiveGoal::kMinimizeReciprocal);
        EXPECT_EQ(tModifiedF.evaluate<core::evaluation::kFunction>(tX), 1.0 / tUnmodifiedValue);
        EXPECT_EQ(tModifiedF.evaluate<core::evaluation::kFirstDerivative>(tX),
                  -1.0 / tUnmodifiedValue / tUnmodifiedValue * tUnmodifiedDerivative);
    }
}
}  // namespace plato::criteria::library::unittest
