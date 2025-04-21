#include <gtest/gtest.h>

#include "plato/filter/library/FilterRegistration.hpp"

namespace plato::filter::extension::unittest
{
TEST(HelmholtzFilter, Registration) { EXPECT_TRUE(library::is_filter_function_registered("helmholtz_filter")); }
}  // namespace plato::filter::extension::unittest
