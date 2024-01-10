#include <gtest/gtest.h>

#include "Plato_SuppressBoostNvccWarnings.hpp"

#include "Plato_InputBlockStruct.hpp"
#include "Plato_Test_Helpers.hpp"

PLATO_NAMED_INPUT_BLOCK_STRUCT(
    (Plato), TestNamedBlock,
    (int, field1)
    (double, field2)
)

TEST(InputBlockStruct, Named)
{
    // These are mostly compile-time checks
    Plato::TestNamedBlock tTestBlock;
    tTestBlock.name = "rockemsockem";
    tTestBlock.field1 = 42;
    tTestBlock.field2 = 0.5;
    Plato::Test::test_existence_and_equality(tTestBlock.name, "rockemsockem");
    Plato::Test::test_existence_and_equality(tTestBlock.field1, 42);
    Plato::Test::test_existence_and_equality(tTestBlock.field2, 0.5);
    EXPECT_TRUE(Plato::Input::kIsNamedBlock<Plato::TestNamedBlock>);
    EXPECT_EQ(Plato::Input::InputTypeName<Plato::TestNamedBlock>::name, "TestNamedBlock");
}

PLATO_INPUT_BLOCK_STRUCT(
    (Plato), TestUnnamedBlock,
    (bool, field1)
    (int, field2)
    (double, field3)
)

TEST(InputBlockStruct, UnNamed)
{
    // These are mostly compile-time checks
    Plato::TestUnnamedBlock tTestBlock;
    tTestBlock.field1 = true;
    tTestBlock.field2 = 42;
    tTestBlock.field3 = 0.5;
    Plato::Test::test_existence_and_equality(tTestBlock.field1, true);
    Plato::Test::test_existence_and_equality(tTestBlock.field2, 42);
    Plato::Test::test_existence_and_equality(tTestBlock.field3, 0.5);
    EXPECT_FALSE(Plato::Input::kIsNamedBlock<Plato::TestUnnamedBlock>);
    EXPECT_EQ(Plato::Input::InputTypeName<Plato::TestUnnamedBlock>::name, "TestUnnamedBlock");
}

#include "Plato_RestoreBoostNvccWarnings.hpp"
