#include <gtest/gtest.h>

#include <boost/optional/optional_io.hpp>

#include "plato/input_parser/InputBlockStruct.hpp"
#include "plato/input_parser/InputBlockUtilities.hpp"
#include "plato/input_parser/ParsedInput.hpp"
#include "plato/test_utilities/TestContext.hpp"

// clang-format off
PLATO_GEOMETRY_INPUT_BLOCK_STRUCT(
    (plato)(input_parser), TestGeometryBlock, 
    (int, field1, "help")
)
PLATO_PROCESS_MANAGER_INPUT_BLOCK_STRUCT(
    (plato)(input_parser), TestProcessManagerBlock, 
    (int, field1, "help")
)
PLATO_FILTER_INPUT_BLOCK_STRUCT(
    (plato)(input_parser), TestFilterBlock, 
    (int, field1, "help")
)
PLATO_NAMED_INPUT_BLOCK_STRUCT(
    (plato)(input_parser), TestObjectiveBlock,
    plato::input_parser::ComponentType::kObjective,
    (int, field1, "help")
)
PLATO_NAMED_INPUT_BLOCK_STRUCT(
    (plato)(input_parser), TestConstraintBlock,
    plato::input_parser::ComponentType::kConstraint,
    (int, field1, "help")
)
// clang-format on

namespace plato::input_parser::unittest
{
namespace
{
template <ComponentType kComponentType, typename InputBlock>
void check_input_block(const NewParsedInput& aFullInput,
                       const InputBlock& aExpectedBlock,
                       const test_utilities::TestContext& aTestContext)
{
    const auto& tComponent = aFullInput.get<kComponentType>();
    ASSERT_EQ(tComponent.size(), 1U) << aTestContext;
    EXPECT_EQ(tComponent.front().mComponentType, kComponentType) << aTestContext;
    EXPECT_EQ(tComponent.front().mBlockName, block_name<InputBlock>()) << aTestContext;
    ASSERT_TRUE(tComponent.front().mInput.template holds_expected_type<InputBlock>()) << aTestContext;
    EXPECT_EQ(tComponent.front().mInput.template get<InputBlock>().field1, aExpectedBlock.field1) << aTestContext;
    if constexpr (kIsNamedComponent<kComponentType>)
    {
        EXPECT_EQ(tComponent.front().mInput.template get<InputBlock>().name, aExpectedBlock.name) << aTestContext;
    }
}
}  // namespace

TEST(InputBlockUtilities, PipeOperator)
{
    const auto tGeometryBlock = TestGeometryBlock{/*.field1=*/42};
    const auto tProcessManagerBlock = TestProcessManagerBlock{/*.field1=*/43};
    const auto tCombinedInput = NewParsedInput{{}} | tGeometryBlock | tProcessManagerBlock;

    check_input_block<ComponentType::kGeometry>(tCombinedInput, tGeometryBlock, TEST_CONTEXT("Geometry block"));
    check_input_block<ComponentType::kProcessManager>(tCombinedInput, tProcessManagerBlock,
                                                      TEST_CONTEXT("Process manager block"));

    EXPECT_TRUE(tCombinedInput.get<ComponentType::kFilter>().empty());
    EXPECT_TRUE(tCombinedInput.get<ComponentType::kObjective>().empty());
    EXPECT_TRUE(tCombinedInput.get<ComponentType::kConstraint>().empty());
}

TEST(InputBlockUtilities, PipeOperatorNamedTypes)
{
    const auto tFilterBlock = TestFilterBlock{/*.field1=*/40};
    const auto tObjectiveBlock = TestObjectiveBlock{/*.name=*/std::string{"bob"}, /*.field1=*/41};
    const auto tConstraintBlock = TestConstraintBlock{/*.name=*/std::string{"sally"}, /*.field1=*/42};
    const auto tCombinedInput = NewParsedInput{{}} | tFilterBlock | tObjectiveBlock | tConstraintBlock;

    check_input_block<ComponentType::kFilter>(tCombinedInput, tFilterBlock, TEST_CONTEXT("Filterblock"));
    check_input_block<ComponentType::kObjective>(tCombinedInput, tObjectiveBlock, TEST_CONTEXT("Objective block"));
    check_input_block<ComponentType::kConstraint>(tCombinedInput, tConstraintBlock, TEST_CONTEXT("Constraint block"));

    EXPECT_TRUE(tCombinedInput.get<ComponentType::kGeometry>().empty());
    EXPECT_TRUE(tCombinedInput.get<ComponentType::kProcessManager>().empty());
}

}  // namespace plato::input_parser::unittest
