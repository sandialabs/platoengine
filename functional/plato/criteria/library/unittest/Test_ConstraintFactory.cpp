#include <gtest/gtest.h>

#include "plato/criteria/library/ConstraintFactory.hpp"

namespace plato::criteria::library::unittest
{

TEST(ConstraintFactory, SizedDualVector)
{
    constexpr unsigned int tSize = 10;
    const linear_algebra::DynamicVector<double> tDualVector = criteria::library::make_dual_vector(tSize);
    ASSERT_EQ(tDualVector.size(), tSize);
    EXPECT_EQ(tDualVector.stdVector().front(), 1.0);
    EXPECT_EQ(tDualVector.stdVector().back(), 1.0);
}

}  // namespace plato::criteria::library::unittest
