#include <gtest/gtest.h>

#include "plato/criteria/library/ConstraintTarget.hpp"

namespace plato::criteria::library::unittest
{
TEST(ConstraintTarget, Size)
{
    {
        const auto tConstraintTarget = ConstraintTarget{42.0};
        EXPECT_EQ(tConstraintTarget.size(), 1U);
    }
    {
        const auto tConstraintTarget = ConstraintTarget{{42.0, 43.0}};
        EXPECT_EQ(tConstraintTarget.size(), 2U);
    }
}

TEST(ConstraintTarget, Value)
{
    // Single-value constraint target, scalar constraint
    {
        constexpr auto tTargetValue = 42.0;
        const auto tConstraintTarget = ConstraintTarget{tTargetValue};
        constexpr auto tConstraintDimension = 1U;
        const auto tExpected = std::vector{tTargetValue};
        EXPECT_EQ(tConstraintTarget.value(tConstraintDimension), tExpected);
    }
    // Single-value constraint target, vector constraint
    {
        constexpr auto tTargetValue = 42.0;
        const auto tConstraintTarget = ConstraintTarget{tTargetValue};
        constexpr auto tConstraintDimension = 3U;
        const auto tExpected = std::vector<double>(tConstraintDimension, tTargetValue);
        EXPECT_EQ(tConstraintTarget.value(tConstraintDimension), tExpected);
    }
    // Vector-valued constraint target, vector constraint
    {
        const auto tTargetValues = std::vector{42.0, 43.0};
        const auto tConstraintTarget = ConstraintTarget{tTargetValues};
        constexpr auto tConstraintDimension = 2U;
        EXPECT_EQ(tConstraintTarget.value(tConstraintDimension), tTargetValues);
    }
}

}  // namespace plato::criteria::library::unittest
