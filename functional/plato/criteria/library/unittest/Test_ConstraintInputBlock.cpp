#include <gtest/gtest.h>

#include "plato/criteria/library/ConstraintInputBlock.hpp"
#include "plato/input_parser/test_utilities/ParseSingleInputBlock.hpp"
#include "plato/input_parser/test_utilities/TestHelpers.hpp"

namespace plato::criteria::library::unittest
{
TEST(ConstraintInputBlock, EnumTable)
{
    EXPECT_TRUE(input_parser::kConstraintTypesTable.toString(input_parser::ConstraintTypes::kEqualTo));
    EXPECT_EQ(input_parser::kConstraintTypesTable.toString(input_parser::ConstraintTypes::kEqualTo).value(),
              "equal_to");

    EXPECT_TRUE(input_parser::kConstraintTypesTable.toString(input_parser::ConstraintTypes::kLessThan));
    EXPECT_EQ(input_parser::kConstraintTypesTable.toString(input_parser::ConstraintTypes::kLessThan).value(),
              "less_than");

    EXPECT_TRUE(input_parser::kConstraintTypesTable.toString(input_parser::ConstraintTypes::kGreaterThan));
    EXPECT_EQ(input_parser::kConstraintTypesTable.toString(input_parser::ConstraintTypes::kGreaterThan).value(),
              "greater_than");
}

TEST(ConstraintInputBlock, ConstraintValueList)
{
    constexpr auto tConstraintInput = std::string_view{
        "component pumpkin target 42.0,\n"
        "component ghost target 84.0\n"};

    const auto [tResult, tParsedSuccessfully] =
        input_parser::test_utilities::parse_input<ConstraintValueList>(tConstraintInput);

    EXPECT_TRUE(tParsedSuccessfully);
    ASSERT_EQ(tResult.mList.size(), 2U);

    EXPECT_EQ(tResult.mList.at(0U).component.mToken, "pumpkin");
    EXPECT_EQ(tResult.mList.at(0U).target, 42.0);
    EXPECT_EQ(tResult.mList.at(1U).component.mToken, "ghost");
    EXPECT_EQ(tResult.mList.at(1U).target, 84.0);
}

TEST(ConstraintInputBlock, FullBlockParse)
{
    constexpr auto tConstraintInput = std::string_view{
        "begin constraint halloween\n"
        "  active true\n"
        "  app october\n"
        "  criterion thirty_first\n"
        "  number_of_processors 666\n"
        "  input_files jack, o, lantern\n"
        "  constraint_value 42.0\n"
        "  constraint_value_list component skeleton target 0.0,\n"
        "                        component frankenstein target -3.0\n"
        "  is_linear false\n"
        "  constraint_type equal_to\n"
        "end"};

    const auto tParsedResult = input_parser::test_utilities::parse_single_input_block<
        input_parser::constraint, components::ComponentType::kConstraint>(tConstraintInput);
    ASSERT_TRUE(tParsedResult.hasValue()) << tParsedResult.error();

    const auto& tParsedData = tParsedResult.value();

    EXPECT_TRUE(tParsedData.active.value());
    EXPECT_EQ(tParsedData.app->mToken, "october");
    EXPECT_EQ(tParsedData.criterion->mToken, "thirty_first");
    EXPECT_EQ(tParsedData.number_of_processors.value(), 666);
    EXPECT_EQ(tParsedData.input_files->size(), 3U);
    EXPECT_EQ(tParsedData.constraint_value.value(), 42.0);
    EXPECT_EQ(tParsedData.constraint_value_list->mList.size(), 2U);
    EXPECT_FALSE(tParsedData.is_linear.value());
    EXPECT_EQ(tParsedData.constraint_type.value(), input_parser::ConstraintTypes::kEqualTo);
}
}  // namespace plato::criteria::library::unittest
