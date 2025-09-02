#include <gtest/gtest.h>

#include "plato/utilities/LogSpaceGenerator.hpp"

namespace plato::utilities::unittest
{
TEST(LogSpaceGenerator, CreateLogSpace)
{
    const LogSpaceGenerator tGenerator{16, 0.5, 5};
    const std::vector<double> tResult = tGenerator.steps();
    const std::vector<double> tGold = {16, 8, 4, 2, 1};
    EXPECT_EQ(tResult, tGold);
}

TEST(LogSpaceGenerator, ZeroSteps)
{
    const LogSpaceGenerator tGenerator{16, 0.5, 0};
    EXPECT_TRUE(tGenerator.steps().empty());
}

TEST(LogSpaceGenerator, OneStep)
{
    const LogSpaceGenerator tGenerator{16, 0.5, 1};
    const std::vector<double> tGold = {16};
    EXPECT_EQ(tGenerator.steps(), tGold);
}

}  // namespace plato::utilities::unittest
