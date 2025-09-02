#include <gtest/gtest.h>

#include <cstddef>

#include "plato/process_manager/extension/CombinationGenerator.hpp"

namespace plato::process_manager::extension::unittest
{
namespace
{
const auto kStepsVector = std::vector<std::vector<double>>{{0.1, 0.2, 0.3}, {-0.4, -0.2}};
const auto kSubIndexVector = std::vector<std::size_t>{2U, 4U};
const auto kOriginalVector = std::vector<double>{1, 2, 3, 4, 5};

const auto kCombinationGenerator = CombinationGenerator{kStepsVector, SubIndexVector{kSubIndexVector}, kOriginalVector};

}  // namespace

TEST(CombinationGenerator, Begin)
{
    auto tCombinationGenerator = kCombinationGenerator;
    const auto tResult = *tCombinationGenerator.begin();
    const auto tGold = std::vector<double>{1, 2, 0.1, 4, -0.4};
    EXPECT_EQ(tGold, tResult);
}

TEST(CombinationGenerator, Transform)
{
    std::vector<std::vector<double>> tStates;
    tStates.reserve(kStepsVector.front().size() * kStepsVector.back().size());
    auto tCombinationGenerator = kCombinationGenerator;

    std::transform(tCombinationGenerator.begin(), tCombinationGenerator.end(), std::back_inserter(tStates),
                   [](const auto aCombination) { return aCombination; });

    ASSERT_EQ(tStates.size(), kStepsVector.front().size() * kStepsVector.back().size());

    {
        const auto tGold = std::vector<double>{1, 2, 0.1, 4, -0.4};
        EXPECT_EQ(tStates.at(0), tGold);
    }
    {
        const auto tGold = std::vector<double>{1, 2, 0.3, 4, -0.4};
        EXPECT_EQ(tStates.at(2), tGold);
    }
    {
        const auto tGold = std::vector<double>{1, 2, 0.1, 4, -0.2};
        EXPECT_EQ(tStates.at(3), tGold);
    }
}

TEST(CombinationGeneratorDetail, SizesVector)
{
    const auto tResult = detail::sizes_vector(kStepsVector).mValue;
    const auto tGold = std::vector<std::size_t>{3, 2};
    EXPECT_EQ(tResult, tGold);
}

TEST(CombinationGeneratorDetail, StateValues)
{
    {
        const auto tResult = detail::state_values(kOriginalVector, kStepsVector, SubIndexVector{kSubIndexVector},
                                                  IndexStateVector{{0, 0}});
        const auto tGold = std::vector<double>{kOriginalVector[0], kOriginalVector[1], kStepsVector[0][0],
                                               kOriginalVector[3], kStepsVector[1][0]};
        EXPECT_EQ(tResult, tGold);
    }
    {
        const auto tResult = detail::state_values(kOriginalVector, kStepsVector, SubIndexVector{kSubIndexVector},
                                                  IndexStateVector{{2, 1}});
        const auto tGold = std::vector<double>{kOriginalVector[0], kOriginalVector[1], kStepsVector[0][2],
                                               kOriginalVector[3], kStepsVector[1][1]};
        EXPECT_EQ(tResult, tGold);
    }
}

}  // namespace plato::process_manager::extension::unittest
