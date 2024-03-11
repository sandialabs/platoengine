#include <gtest/gtest.h>

#include "plato/utilities/ParameterBounds.hpp"
namespace plato::utilities::unittest
{
TEST(ParameterBounds, DirectConstructionAllInclusive)
{
    const auto tBounds = ParameterBounds{Inclusive{0.0}, Inclusive{1.0}};
    EXPECT_FALSE(tBounds.contains(-0.5));
    EXPECT_TRUE(tBounds.contains(0.0));
    EXPECT_TRUE(tBounds.contains(0.5));
    EXPECT_TRUE(tBounds.contains(1.0));
    EXPECT_FALSE(tBounds.contains(1.5));
}

TEST(ParameterBounds, DirectConstructionAllExclusive)
{
    const auto tBounds = ParameterBounds{Exclusive{0.0}, Exclusive{1.0}};
    EXPECT_FALSE(tBounds.contains(-0.5));
    EXPECT_FALSE(tBounds.contains(0.0));
    EXPECT_TRUE(tBounds.contains(0.5));
    EXPECT_FALSE(tBounds.contains(1.0));
    EXPECT_FALSE(tBounds.contains(1.5));
}

TEST(ParameterBounds, DirectConstructionInclusiveLowerExclusiveUpper)
{
    const auto tBounds = ParameterBounds{Inclusive{0.25}, Exclusive{0.5}};
    EXPECT_FALSE(tBounds.contains(-0.5));
    EXPECT_TRUE(tBounds.contains(0.25));
    EXPECT_TRUE(tBounds.contains(0.3));
    EXPECT_FALSE(tBounds.contains(0.5));
    EXPECT_FALSE(tBounds.contains(2.5));
}

TEST(ParameterBounds, DirectConstructionExclusiveLowerInclusiveUpper)
{
    const auto tBounds = ParameterBounds{Exclusive{-10.0}, Inclusive{1.0}};
    EXPECT_FALSE(tBounds.contains(-11.0));
    EXPECT_FALSE(tBounds.contains(-10.0));
    EXPECT_TRUE(tBounds.contains(0.5));
    EXPECT_TRUE(tBounds.contains(1.0));
    EXPECT_FALSE(tBounds.contains(1.5));
}

TEST(ParameterBounds, Unbounded)
{
    const auto tBounds = unbounded<double>();
    EXPECT_TRUE(tBounds.contains(-11.0));
    EXPECT_TRUE(tBounds.contains(-10.0));
    EXPECT_TRUE(tBounds.contains(0.5));
    EXPECT_TRUE(tBounds.contains(1.0));
    EXPECT_TRUE(tBounds.contains(1.5));
}

TEST(ParameterBounds, LowerBounded)
{
    const auto tBounds = lower_bounded(Inclusive{0.0});
    EXPECT_FALSE(tBounds.contains(-11.0));
    EXPECT_TRUE(tBounds.contains(0.0));
    EXPECT_TRUE(tBounds.contains(1.5));
}

TEST(ParameterBounds, UpperBounded)
{
    const auto tBounds = upper_bounded(Exclusive{0.0});
    EXPECT_TRUE(tBounds.contains(-11.0));
    EXPECT_FALSE(tBounds.contains(0.0));
    EXPECT_FALSE(tBounds.contains(1.5));
}

TEST(ParameterBounds, DirectConstructionExclusiveLowerInclusiveUpperInt)
{
    const auto tBounds = ParameterBounds{Exclusive{0}, Inclusive{10}};
    EXPECT_FALSE(tBounds.contains(-11.0));
    EXPECT_FALSE(tBounds.contains(0));
    EXPECT_TRUE(tBounds.contains(5));
    EXPECT_TRUE(tBounds.contains(10));
    EXPECT_FALSE(tBounds.contains(15));
}

TEST(ParameterBounds, DirectConstructionExclusiveLowerInclusiveUpperUnsignedInt)
{
    const auto tBounds = ParameterBounds{Exclusive{0u}, Inclusive{10u}};
    EXPECT_FALSE(tBounds.contains(0));
    EXPECT_TRUE(tBounds.contains(5));
    EXPECT_TRUE(tBounds.contains(10));
    EXPECT_FALSE(tBounds.contains(15));
}

TEST(ParameterBounds, Description)
{
    EXPECT_EQ(unbounded<double>().description(), "(-inf, inf)");
    EXPECT_EQ((ParameterBounds{Inclusive{0}, Inclusive{10}}.description()), "[0, 10]");
    EXPECT_EQ((ParameterBounds{Exclusive{-42}, Exclusive{0}}.description()), "(-42, 0)");
}
}  // namespace plato::utilities::unittest