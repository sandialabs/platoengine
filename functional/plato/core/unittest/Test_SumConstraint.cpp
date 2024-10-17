#include <gtest/gtest.h>

#include "plato/test_utilities/SumConstraint.hpp"

namespace plato::core::unittest
{
TEST(SumConstraint, Linear)
{
    const test_utilities::SumConstraint tSumConstraint;
    EXPECT_EQ(tSumConstraint.f(1.0, 1.0), 2.0);
    EXPECT_EQ(tSumConstraint.f(2.0, 0.0), 2.0);
    EXPECT_EQ(tSumConstraint.df(1.0, 2.0)(0), 1.0);
    EXPECT_EQ(tSumConstraint.df(2.0, 1.0)(1), 1.0);
    EXPECT_EQ(tSumConstraint.df(14.0, 32.0)(0), 1.0);
    EXPECT_EQ(tSumConstraint.df(24.0, 31.0)(1), 1.0);
}

TEST(SumConstraint, Quadratic)
{
    constexpr std::pair<double, double> tCenter{1, 2};
    constexpr test_utilities::SumConstraint tCircleConstraint{tCenter, 2};
    EXPECT_EQ(tCircleConstraint.f(1.0, 2.0), 0);
    EXPECT_DOUBLE_EQ(tCircleConstraint.f(2.0, 3.0), 2.0);
    EXPECT_EQ(tCircleConstraint.df(2.0, 3.0)(0), 2.0);
    EXPECT_EQ(tCircleConstraint.df(2.0, 3.0)(1), 2.0);
}

}  // namespace plato::core::unittest
