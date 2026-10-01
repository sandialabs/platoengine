#include <gtest/gtest.h>

#include <boost/fusion/sequence/intrinsic/size.hpp>
#include <boost/fusion/support/is_sequence.hpp>

#include "plato/components/ComponentType.hpp"
#include "plato/input_parser/InputBlockStruct.hpp"
#include "plato/input_parser/test_utilities/TestHelpers.hpp"

// clang-format off
PLATO_NAMED_INPUT_BLOCK_STRUCT(
    (plato)(input_parser), TestNamedBlock, plato::components::ComponentType::kConstraint,
    "A test constraint",
    (int, field1, "help")
    (double, field2, "help")
)

PLATO_INPUT_BLOCK_STRUCT(
    (plato)(input_parser), TestUnnamedBlock, plato::components::ComponentType::kFilter,
    "A test filter",
    (bool, field1, "a helpful comment")
    (int, field2, "only sorta helpful")
    (double, field3, "did we even read this part?")
)

PLATO_GEOMETRY_INPUT_BLOCK_STRUCT(
    (plato)(input_parser), TestGeometryBlock, 
    "Description",
    (bool, field1, "help")
)

PLATO_PROCESS_MANAGER_INPUT_BLOCK_STRUCT(
    (plato)(input_parser), TestProcessManagerBlock, 
    "Description",
    (int, field1, "help")
)

PLATO_FILTER_INPUT_BLOCK_STRUCT(
    (plato)(input_parser), TestFilterBlock, 
    "Description",
    (double, field1, "help")
)

PLATO_FILTER_INPUT_BLOCK_STRUCT(
    (plato)(input_parser), TestFilterBlockNoFields, 
    "Description"
)

PLATO_NAMED_INPUT_BLOCK_STRUCT(
    (plato)(input_parser), TestNamedBlockNoFields, plato::components::ComponentType::kConstraint,
    "A test constraint"
)
// clang-format on

namespace plato::input_parser::unittest
{
namespace
{
constexpr auto kArbitraryInt = 42;
constexpr auto kArbitraryDouble = 0.5;
}  // namespace

TEST(InputBlockStruct, Named)
{
    // These are mostly compile-time checks
    TestNamedBlock tTestBlock;
    tTestBlock.name = "rockemsockem";
    tTestBlock.field1 = kArbitraryInt;
    tTestBlock.field2 = kArbitraryDouble;
    test_utilities::test_existence_and_equality(tTestBlock.name, "rockemsockem");
    test_utilities::test_existence_and_equality(tTestBlock.field1, kArbitraryInt);
    test_utilities::test_existence_and_equality(tTestBlock.field2, kArbitraryDouble);
    EXPECT_TRUE(kIsNamedBlock<TestNamedBlock>);
    EXPECT_EQ(InputTypeName<TestNamedBlock>::mName, "TestNamedBlock");
}

TEST(InputBlockStruct, UnNamed)
{
    // These are mostly compile-time checks
    TestUnnamedBlock tTestBlock;
    tTestBlock.field1 = true;
    tTestBlock.field2 = kArbitraryInt;
    tTestBlock.field3 = kArbitraryDouble;
    test_utilities::test_existence_and_equality(tTestBlock.field1, true);
    test_utilities::test_existence_and_equality(tTestBlock.field2, kArbitraryInt);
    test_utilities::test_existence_and_equality(tTestBlock.field3, kArbitraryDouble);
    EXPECT_FALSE(kIsNamedBlock<TestUnnamedBlock>);
    EXPECT_EQ(InputTypeName<TestUnnamedBlock>::mName, "TestUnnamedBlock");
}

TEST(InputBlockStruct, ComponentTypeOfInputBlock)
{
    EXPECT_EQ(ComponentTypeOfInputBlock<TestGeometryBlock>::mValue, components::ComponentType::kGeometry);
    EXPECT_EQ(ComponentTypeOfInputBlock<TestProcessManagerBlock>::mValue, components::ComponentType::kProcessManager);
    EXPECT_EQ(ComponentTypeOfInputBlock<TestFilterBlock>::mValue, components::ComponentType::kFilter);
}

TEST(InputBlockStruct, ProcessManager)
{
    auto tTestBlock = TestProcessManagerBlock{};
    tTestBlock.field1 = kArbitraryInt;
    test_utilities::test_existence_and_equality(tTestBlock.field1, kArbitraryInt);
}

TEST(InputBlockStruct, FilterNoFields)
{
    using InputBlock = plato::input_parser::TestFilterBlockNoFields;
    static_assert(boost::fusion::traits::is_sequence<InputBlock>::value);
    static_assert(boost::fusion::result_of::size<InputBlock>::value == 0);
}

TEST(InputBlockStruct, NamedStructNoFields)
{
    TestNamedBlockNoFields tTestBlock;
    tTestBlock.name = "icarus";
    test_utilities::test_existence_and_equality(tTestBlock.name, "icarus");
    EXPECT_TRUE(kIsNamedBlock<TestNamedBlockNoFields>);
    EXPECT_EQ(InputTypeName<TestNamedBlockNoFields>::mName, "TestNamedBlockNoFields");
}

}  // namespace plato::input_parser::unittest
