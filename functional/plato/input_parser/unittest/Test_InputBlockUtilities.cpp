#include <gtest/gtest.h>

#include <boost/optional/optional_io.hpp>

#include "plato/input_parser/InputBlockStruct.hpp"
#include "plato/input_parser/InputBlockUtilities.hpp"
#include "plato/input_parser/ParsedInput.hpp"

// clang-format off
PLATO_GEOMETRY_INPUT_BLOCK_STRUCT(
    (plato)(input_parser), TestGeometryBlock, 
    (int, field1, "help")
)
PLATO_PROCESS_MANAGER_INPUT_BLOCK_STRUCT(
    (plato)(input_parser), TestProcessManagerBlock, 
    (int, field1, "help")
)
// clang-format on

namespace plato::input_parser::unittest
{
TEST(InputBlockUtilities, PipeOperator)
{
    const auto tCombinedInput =
        NewParsedInput{{}} | TestGeometryBlock{/*.field1=*/42} | TestProcessManagerBlock{/*.field1=*/43};

    const auto& tGeometries = tCombinedInput.get<ComponentType::kGeometry>();
    ASSERT_EQ(tGeometries.size(), 1U);
    EXPECT_EQ(tGeometries.front().mComponentType, ComponentType::kGeometry);
    EXPECT_EQ(tGeometries.front().mBlockName, block_name<TestGeometryBlock>());
    ASSERT_TRUE(tGeometries.front().mInput.holds_expected_type<TestGeometryBlock>());
    EXPECT_EQ(tGeometries.front().mInput.get<TestGeometryBlock>().field1, 42);

    const auto& tProcessManagers = tCombinedInput.get<ComponentType::kProcessManager>();
    ASSERT_EQ(tProcessManagers.size(), 1U);
    EXPECT_EQ(tProcessManagers.front().mComponentType, ComponentType::kProcessManager);
    EXPECT_EQ(tProcessManagers.front().mBlockName, block_name<TestProcessManagerBlock>());
    ASSERT_TRUE(tProcessManagers.front().mInput.holds_expected_type<TestProcessManagerBlock>());
    EXPECT_EQ(tProcessManagers.front().mInput.get<TestProcessManagerBlock>().field1, 43);

    EXPECT_TRUE(tCombinedInput.get<ComponentType::kFilter>().empty());
    EXPECT_TRUE(tCombinedInput.get<ComponentType::kObjective>().empty());
    EXPECT_TRUE(tCombinedInput.get<ComponentType::kConstraint>().empty());
}
}  // namespace plato::input_parser::unittest
