#include <gtest/gtest.h>

#include "plato/utilities/LinearSpaceGenerator.hpp"
#include "plato/utilities/Zip.hpp"

namespace plato::utilities::unittest
{
TEST(LinearSpaceGenerator, CreateLinearSpace)
{
    const LinearSpaceGenerator tGenerator{.mStartingValue = 1, .mStep = 1, .mEndingValue = 4};
    const std::vector<double> tResult = tGenerator.steps();
    const std::vector<double> tGold = {1, 2, 3, 4};
    EXPECT_EQ(tResult, tGold);
}

TEST(LinearSpaceGenerator, OneStep)
{
    const LinearSpaceGenerator tGenerator{.mStartingValue = 1, .mStep = 5, .mEndingValue = 1};
    const std::vector<double> tGold = {1};
    EXPECT_EQ(tGenerator.steps(), tGold);
}

TEST(LinearSpaceGenerator, TwoStep)
{
    const LinearSpaceGenerator tGenerator{.mStartingValue = 1, .mStep = 1, .mEndingValue = 2};
    const std::vector<double> tGold = {1, 2};
    EXPECT_EQ(tGenerator.steps(), tGold);
}

TEST(LinearSpaceGenerator, Quarters)
{
    const LinearSpaceGenerator tGenerator{.mStartingValue = 0, .mStep = 0.25, .mEndingValue = 1};
    const std::vector<double> tResult = tGenerator.steps();
    const std::vector<double> tGold = {0, .25, 0.5, 0.75, 1};
    ASSERT_EQ(tResult.size(), tGold.size());

    for (unsigned int iIndex = 0; iIndex < tGold.size(); iIndex++)
    {
        EXPECT_DOUBLE_EQ(tResult[iIndex], tGold[iIndex]);
    }
}

TEST(LinearSpaceGenerator, IncomensurateStepSize)
{
    const LinearSpaceGenerator tGenerator{.mStartingValue = 2.5, .mStep = 0.15, .mEndingValue = 3.5};
    const std::vector<double> tGoldVector = {2.5, 2.65, 2.8, 2.95, 3.1, 3.25, 3.4};
    for (const auto& [tGold, tResult] : utilities::Zip(tGoldVector, tGenerator.steps()))
    {
        EXPECT_DOUBLE_EQ(tGold, tResult);
    }
}

}  // namespace plato::utilities::unittest
