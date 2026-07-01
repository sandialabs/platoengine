#include <gtest/gtest.h>

#include "plato/documentation/DataTypeDocumentation.hpp"

namespace plato::documentation::unittest
{
TEST(DataTypeDocumentation, Double) { EXPECT_EQ(data_type_documentation(double{}), "`<real>`"); }

TEST(DataTypeDocumentation, Int) { EXPECT_EQ(data_type_documentation(int{}), "`<int>`"); }

TEST(DataTypeDocumentation, UnsignedInt) { EXPECT_EQ(data_type_documentation(unsigned{}), "`<unsigned int>`"); }

TEST(DataTypeDocumentation, Bool) { EXPECT_EQ(data_type_documentation(bool{}), "`true | false`"); }

}  // namespace plato::documentation::unittest
