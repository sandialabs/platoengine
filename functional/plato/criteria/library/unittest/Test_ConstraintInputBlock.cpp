#include <gtest/gtest.h>

#include "plato/criteria/library/ConstraintInputBlock.hpp"

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
}  // namespace plato::criteria::library::unittest
