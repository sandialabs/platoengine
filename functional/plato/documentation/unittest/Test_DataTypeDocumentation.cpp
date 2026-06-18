#include <gtest/gtest.h>

#include "plato/documentation/DataTypeDocumentation.hpp"

namespace plato::documentation::unittest
{
TEST(DataTypeDocumentation, Double) { EXPECT_EQ(data_type_documentation(double{}), "Floating point"); }

TEST(DataTypeDocumentation, Int) { EXPECT_EQ(data_type_documentation(int{}), "Signed integer"); }

TEST(DataTypeDocumentation, UnsignedInt) { EXPECT_EQ(data_type_documentation(unsigned{}), "Unsigned integer"); }

TEST(DataTypeDocumentation, Bool) { EXPECT_EQ(data_type_documentation(bool{}), "`true`|`false`"); }

}  // namespace plato::documentation::unittest
