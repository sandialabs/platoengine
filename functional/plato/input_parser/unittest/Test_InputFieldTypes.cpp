#include <gtest/gtest.h>

#include "plato/input_parser/InputFieldTypes.hpp"

namespace plato::input_parser::unittest
{
TEST(InputFieldTypes, DataTypeNames)
{
    EXPECT_EQ(data_type_documentation(AppName{}), "String");
    EXPECT_EQ(data_type_documentation(CriterionName{}), "String");
    EXPECT_EQ(data_type_documentation(BlockName{}), "String");
    EXPECT_EQ(data_type_documentation(BlockList{}), "Comma separated list of strings");
    EXPECT_EQ(data_type_documentation(IdentifierString{}), "String");
}

}  // namespace plato::input_parser::unittest
