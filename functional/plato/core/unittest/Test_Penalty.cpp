#include <gtest/gtest.h>

#include "plato/core/test_utilities/Utilities.hpp"
#include "plato/test_utilities/Penalty.hpp"

namespace plato::core::unittest
{
TEST(Penalty, Penalty)
{
    namespace pft = plato::test_utilities;

    constexpr auto tXMin = pft::XMin{0.5e-2};
    constexpr auto tExponent = pft::Exponent{2.0};
    const auto tPenalty = pft::Penalty{tXMin, tExponent};

    // Function and derivative at 1 and 0
    const pft::TwoDVector tFx = tPenalty.f(1.0, 0.0);
    EXPECT_EQ(tFx(0), 1.0);
    EXPECT_EQ(tFx(1), tXMin.mValue);

    const pft::TwoDMatrix tDfx = tPenalty.df(1.0, 0.0);
    EXPECT_EQ(tDfx(0, 0), tExponent.mValue * (1.0 - tXMin.mValue));
    EXPECT_EQ(tDfx(0, 1), 0.0);
    EXPECT_EQ(tDfx(1, 0), 0.0);
    EXPECT_EQ(tDfx(1, 1), 0.0);
}

TEST(Penalty, MakePenalty)
{
    namespace pft = plato::test_utilities;

    constexpr auto tXMin = pft::XMin{0.5e-2};
    constexpr auto tExponent = pft::Exponent{2.0};
    const auto tPenalty = pft::Penalty{tXMin, tExponent};
    const auto tPenaltyFunction = test_utilities::make_penalty_function(tPenalty);

    {
        const auto tX = pft::TwoDVector{0.5, 10.0};
        EXPECT_EQ(tPenalty.f(tX(0), tX(1)), tPenaltyFunction.evaluate<evaluation::kFunction>(tX));
        EXPECT_EQ(tPenalty.df(tX(0), tX(1)), tPenaltyFunction.evaluate<evaluation::kFirstDerivative>(tX));
    }
    {
        const auto tX = pft::TwoDVector{-0.5, 5.0};
        EXPECT_EQ(tPenalty.f(tX(0), tX(1)), tPenaltyFunction.evaluate<evaluation::kFunction>(tX));
        EXPECT_EQ(tPenalty.df(tX(0), tX(1)), tPenaltyFunction.evaluate<evaluation::kFirstDerivative>(tX));
    }
}
}  // namespace plato::core::unittest
