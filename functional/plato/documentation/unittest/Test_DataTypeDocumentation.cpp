#include <gtest/gtest.h>

#include "plato/documentation/DataTypeDocumentation.hpp"

namespace plato::documentation::unittest
{
TEST(DataTypeDocumentation, Double) { EXPECT_EQ(DataTypeDocumentation<double>::mDocumentation, "Floating point"); }

TEST(DataTypeDocumentation, Int) { EXPECT_EQ(DataTypeDocumentation<int>::mDocumentation, "Signed integer"); }

TEST(DataTypeDocumentation, UnsignedInt)
{
    EXPECT_EQ(DataTypeDocumentation<unsigned int>::mDocumentation, "Unsigned integer");
}

TEST(DataTypeDocumentation, Bool) { EXPECT_EQ(DataTypeDocumentation<bool>::mDocumentation, "`true`|`false`"); }

}  // namespace plato::documentation::unittest
