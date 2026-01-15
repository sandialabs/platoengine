#include <gtest/gtest.h>

#include "plato/criteria/library/ObjectiveFactory.hpp"

namespace plato::criteria::library::parallel_unittest
{
TEST(ObjectiveFactory, TotalWeight)
{
    {
        const auto tAggregateData = detail::AggregationData{
            .mName = std::string{"baloney"}, .mWeight = 2.0, .mNormalization = std::nullopt, .mGoalScaling = 4.0};
        EXPECT_EQ(detail::total_weight(tAggregateData), tAggregateData.mWeight * tAggregateData.mGoalScaling);
    }
    {
        const auto tAggregateData = detail::AggregationData{
            .mName = std::string{"baloney"}, .mWeight = 2.0, .mNormalization = 4.0, .mGoalScaling = 4.0};
        EXPECT_EQ(detail::total_weight(tAggregateData),
                  tAggregateData.mWeight * tAggregateData.mGoalScaling / tAggregateData.mNormalization.value());
    }
}
}  // namespace plato::criteria::library::parallel_unittest
