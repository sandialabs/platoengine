#include <gtest/gtest.h>

#include <utility>
#include <vector>

#include "plato/linear_algebra/DynamicVector.hpp"
#include "plato/linear_algebra/JacobianColumnEvaluator.hpp"
#include "plato/linear_algebra/JacobianMultiplier.hpp"

namespace plato::linear_algebra::unittest
{
TEST(JacobianColumnEvaluator, MultiplicationOperatorOverload)
{
    const JacobianColumnEvaluator tJacobian = {/*.mColumns=*/1,
                                               /*.mX=*/DynamicVector<double>{1.0, 2.0, 3.0, 4.0},
                                               /*.mColumnFunction=*/[](unsigned int, const DynamicVector<double>&)
                                               { return DynamicVector<double>{-1.0, -2.0, -3.0, -4.0}; }};

    const DynamicVector<double> tVec{1.0, 2.0, 3.0, 4.0};
    const std::vector<double> tGold{-30};
    const DynamicVector<double> tResult = tVec * tJacobian;
    EXPECT_EQ(tResult.stdVector(), tGold);
}

TEST(JacobianColumnEvaluator, ToJacobianMultiplier)
{
    const JacobianColumnEvaluator tJacobianColumn = {/*.mColumns=*/1,
                                                     /*.mX=*/DynamicVector<double>{1.0, 2.0, 3.0, 4.0},
                                                     /*.mColumnFunction=*/[](unsigned int, const DynamicVector<double>&)
                                                     { return DynamicVector<double>{-1.0, -2.0, -3.0, -4.0}; }};
    const JacobianMultiplier tJacobian = to_jacobian_multiplier(tJacobianColumn);
    const DynamicVector<double> tVec{1.0, 2.0, 3.0, 4.0};
    const std::vector<double> tGold{-30};
    const DynamicVector<double> tResult = tVec * tJacobian;
    EXPECT_EQ(tResult.stdVector(), tGold);
}

}  // namespace plato::linear_algebra::unittest
