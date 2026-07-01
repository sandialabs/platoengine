#include <gtest/gtest.h>

#include "plato/input_parser/InputFieldTypes.hpp"

namespace plato::input_parser::unittest
{
TEST(InputFieldTypes, DataTypeNames)
{
    EXPECT_EQ(data_type_documentation(AppName{}), "`<string>`");
    EXPECT_EQ(data_type_documentation(CriterionName{}), "`<string>`");
    EXPECT_EQ(data_type_documentation(BlockName{}), "`<string>`");
    EXPECT_EQ(data_type_documentation(BlockList{}), "Comma separated list of `<string>`");
    EXPECT_EQ(data_type_documentation(IdentifierString{}), "`<string>`");
}

}  // namespace plato::input_parser::unittest
