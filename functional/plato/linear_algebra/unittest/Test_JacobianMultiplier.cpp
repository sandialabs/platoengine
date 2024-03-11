#include <gtest/gtest.h>

#include "plato/linear_algebra/JacobianMultiplier.hpp"

namespace plato::linear_algebra::unittest
{
TEST(JacobianMultiplier, Identity)
{
    const JacobianMultiplier tIdentityJacobian = {/*.mNumColumns=*/2,
                                                  /*.mJacobianTimesVectorFunction=*/
                                                  [](const DynamicVector<double>& aV) { return aV; }};

    const auto tVec = DynamicVector<double>{1.0, 2.0};
    const DynamicVector<double> tRes = tVec * tIdentityJacobian;
    EXPECT_EQ(tRes.stdVector(), tVec.stdVector());
}

}  // namespace plato::linear_algebra::unittest
