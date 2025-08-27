#include <gtest/gtest.h>

#include "plato/input_parser/InputBlockStruct.hpp"
#include "plato/input_parser/unittest/Test_Helpers.hpp"

// clang-format off
PLATO_NAMED_INPUT_BLOCK_STRUCT(
    (plato)(input_parser), TestNamedBlock, plato::components::ComponentType::kConstraint,
    (int, field1, "help")
    (double, field2, "help")
)

PLATO_INPUT_BLOCK_STRUCT(
    (plato)(input_parser), TestUnnamedBlock, plato::components::ComponentType::kFilter,
    (bool, field1, "a helpful comment")
    (int, field2, "only sorta helpful")
    (double, field3, "did we even read this part?")
)

PLATO_GEOMETRY_INPUT_BLOCK_STRUCT(
    (plato)(input_parser), TestGeometryBlock, 
    (bool, field1, "help")
)

PLATO_PROCESS_MANAGER_INPUT_BLOCK_STRUCT(
    (plato)(input_parser), TestProcessManagerBlock, 
    (int, field1, "help")
)

PLATO_FILTER_INPUT_BLOCK_STRUCT(
    (plato)(input_parser), TestFilterBlock, 
    (double, field1, "help")
)
// clang-format on
namespace plato::input_parser::unittest
{
TEST(InputBlockStruct, Named)
{
    // These are mostly compile-time checks
    TestNamedBlock tTestBlock;
    tTestBlock.name = "rockemsockem";
    tTestBlock.field1 = 42;
    tTestBlock.field2 = 0.5;
    test_existence_and_equality(tTestBlock.name, "rockemsockem");
    test_existence_and_equality(tTestBlock.field1, 42);
    test_existence_and_equality(tTestBlock.field2, 0.5);
    EXPECT_TRUE(kIsNamedBlock<TestNamedBlock>);
    EXPECT_EQ(InputTypeName<TestNamedBlock>::name, "TestNamedBlock");
}

TEST(InputBlockStruct, UnNamed)
{
    // These are mostly compile-time checks
    TestUnnamedBlock tTestBlock;
    tTestBlock.field1 = true;
    tTestBlock.field2 = 42;
    tTestBlock.field3 = 0.5;
    test_existence_and_equality(tTestBlock.field1, true);
    test_existence_and_equality(tTestBlock.field2, 42);
    test_existence_and_equality(tTestBlock.field3, 0.5);
    EXPECT_FALSE(kIsNamedBlock<TestUnnamedBlock>);
    EXPECT_EQ(InputTypeName<TestUnnamedBlock>::name, "TestUnnamedBlock");
}

TEST(InputBlockStruct, ComponentTypeOfInputBlock)
{
    EXPECT_EQ(ComponentTypeOfInputBlock<TestGeometryBlock>::value, components::ComponentType::kGeometry);
    EXPECT_EQ(ComponentTypeOfInputBlock<TestProcessManagerBlock>::value, components::ComponentType::kProcessManager);
    EXPECT_EQ(ComponentTypeOfInputBlock<TestFilterBlock>::value, components::ComponentType::kFilter);
}

TEST(InputBlockStruct, ProcessManager)
{
    auto tTestBlock = TestProcessManagerBlock{};
    tTestBlock.field1 = 42;
    test_existence_and_equality(tTestBlock.field1, 42);
}

}  // namespace plato::input_parser::unittest
