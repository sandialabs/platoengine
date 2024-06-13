#include <gtest/gtest.h>

#include <array>
#include <list>

#include "plato/utilities/PairWiseAccumulate.hpp"

namespace plato::utilities::unittest
{
TEST(PairWiseAccumulate, PairWiseAccumulate)
{
    {
        const std::vector<double> tVector{};
        EXPECT_DOUBLE_EQ(pair_wise_accumulate(tVector), 0);
    }
    {
        const std::vector<double> tVector{1};
        EXPECT_DOUBLE_EQ(pair_wise_accumulate(tVector), 1);
    }
    {
        constexpr int tTransitionInt = 31;
        const std::vector<double> tVector(tTransitionInt, 1.0 / 6.0);
        EXPECT_DOUBLE_EQ(pair_wise_accumulate(tVector), tTransitionInt / 6.0);
    }
    {
        constexpr int tTransitionInt = 34;
        const std::vector<double> tVector(tTransitionInt, 1.0 / 6.0);
        EXPECT_DOUBLE_EQ(pair_wise_accumulate(tVector), tTransitionInt / 6.0);
    }
    {
        const std::vector<double> tVector(600, 1.0 / 6.0);
        EXPECT_DOUBLE_EQ(pair_wise_accumulate(tVector), 100);
    }
    {
        const std::vector<double> tVector(700000, 1.0 / 7.0);
        EXPECT_DOUBLE_EQ(pair_wise_accumulate(tVector), 100000);
    }
}

TEST(PairWiseAccumulate, PairWiseAccumulateFloat)
{
    {
        const std::vector<float> tVector(1000, 1e-16);
        EXPECT_FLOAT_EQ(pair_wise_accumulate(tVector), 1e-13);
    }
}

TEST(PairWiseAccumulate, PairWiseAccumulateList)
{
    {
        const std::list<double> tList(300, 1.0 / 3.0);
        EXPECT_DOUBLE_EQ(pair_wise_accumulate(tList), 100);
    }
}

TEST(PairWiseAccumulate, PairWiseAccumulateArray)
{
    {
        std::array<double, 40> tArray;
        tArray.fill(1);
        EXPECT_DOUBLE_EQ(pair_wise_accumulate(tArray), 40);
    }
}
}  // namespace plato::utilities::unittest
