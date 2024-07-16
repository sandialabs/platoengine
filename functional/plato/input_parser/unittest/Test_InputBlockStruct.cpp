#include <gtest/gtest.h>

#include "plato/input_parser/InputBlockStruct.hpp"
#include "plato/input_parser/unittest/Test_Helpers.hpp"

// clang-format off
PLATO_NAMED_INPUT_BLOCK_STRUCT(
    (plato)(input_parser), TestNamedBlock, 
    (int, field1)
    (double, field2)
)

PLATO_INPUT_BLOCK_STRUCT(
    (plato)(input_parser), TestUnnamedBlock, 
    (bool, field1)
    (int, field2)
    (double, field3)
)

PLATO_GEOMETRY_INPUT_BLOCK_STRUCT(
    (plato)(input_parser), TestGeometryBlock, 
    (bool, field1)
)

PLATO_PROCESS_MANAGER_INPUT_BLOCK_STRUCT(
    (plato)(input_parser), TestProcessManagerBlock, 
    (int, field1)
)

PLATO_FILTER_INPUT_BLOCK_STRUCT(
    (plato)(input_parser), TestFilterBlock, 
    (double, field1)
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

TEST(InputBlockStruct, Geometry)
{
    const TestGeometryBlock tTestBlock;
    constexpr bool tIsGeometry = IsGeometryInput<TestGeometryBlock>::value;
    EXPECT_TRUE(tIsGeometry);
    constexpr bool tIsNotGeometry = IsGeometryInput<int>::value;
    EXPECT_FALSE(tIsNotGeometry);
}

TEST(InputBlockStruct, Process)
{
    auto tTestBlock = TestProcessManagerBlock{};
    constexpr bool tIsProcessManager = IsProcessManagerInput<TestProcessManagerBlock>::value;
    EXPECT_TRUE(tIsProcessManager);
    constexpr bool tIsNotProcessManager = IsProcessManagerInput<TestGeometryBlock>::value;
    EXPECT_FALSE(tIsNotProcessManager);

    tTestBlock.field1 = 42;
    test_existence_and_equality(tTestBlock.field1, 42);
}

TEST(InputBlockStruct, Filter)
{
    const TestFilterBlock tTestBlock;
    constexpr bool tIsFilter = IsFilterInput<TestFilterBlock>::value;
    EXPECT_TRUE(tIsFilter);
    constexpr bool tIsNotFilter = IsFilterInput<TestGeometryBlock>::value;
    EXPECT_FALSE(tIsNotFilter);
}
}  // namespace plato::input_parser::unittest