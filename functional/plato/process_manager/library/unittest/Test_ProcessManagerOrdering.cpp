#include <gtest/gtest.h>

#include <map>
#include <vector>

#include "plato/process_manager/library/StageOrdering.hpp"

namespace plato::process_manager::library::unittest
{
TEST(ProcessManagerOrdering, SingleStage)
{
    const auto tStageMap = std::multimap<RunStage, int>{{RunStage::kValidate, 42}, {RunStage::kValidate, 42}};
    const auto tVectorResult = to_stage_ordered_vector(tStageMap);
    ASSERT_EQ(tVectorResult.size(), tStageMap.size());
    const auto tExpected = std::vector{42, 42};
    EXPECT_EQ(tVectorResult, tExpected);
}

TEST(ProcessManagerOrdering, TwoStages)
{
    const auto tStageMap = std::multimap<RunStage, int>{
        {RunStage::kExecute, 84},
        {RunStage::kValidate, 42},
        {RunStage::kValidate, 42},
    };
    const auto tVectorResult = to_stage_ordered_vector(tStageMap);
    ASSERT_EQ(tVectorResult.size(), tStageMap.size());
    const auto tExpected = std::vector{42, 42, 84};
    EXPECT_EQ(tVectorResult, tExpected);
}

TEST(ProcessManagerOrdering, AllStages)
{
    const auto tStageMap = std::multimap<RunStage, int>{
        {RunStage::kExecute, 84},
        {RunStage::kPostProcess, 126},
        {RunStage::kExecute, 84},
        {RunStage::kValidate, 42},
    };
    const auto tVectorResult = to_stage_ordered_vector(tStageMap);
    ASSERT_EQ(tVectorResult.size(), tStageMap.size());
    const auto tExpected = std::vector{42, 84, 84, 126};
    EXPECT_EQ(tVectorResult, tExpected);
}

}  // namespace plato::process_manager::library::unittest
