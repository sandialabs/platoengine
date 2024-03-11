#include <gtest/gtest.h>

#include "plato/criteria/library/ConstraintFactory.hpp"

namespace plato::criteria::library::unittest
{
TEST(ConstraintFactory, DualVector)
{
    const linear_algebra::DynamicVector<double> tDualVector = criteria::library::make_dual_vector();
    ASSERT_EQ(tDualVector.size(), 1);
    EXPECT_EQ(tDualVector.stdVector().front(), 1.0);
}
}  // namespace plato::criteria::library::unittest
