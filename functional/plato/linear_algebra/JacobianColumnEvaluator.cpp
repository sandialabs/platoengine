#include "plato/linear_algebra/JacobianColumnEvaluator.hpp"

#include "plato/linear_algebra/JacobianMultiplier.hpp"

namespace plato::linear_algebra
{
DynamicVector<double> JacobianColumnEvaluator::column(const unsigned int aIndex) const
{
    return mColumnFunction(aIndex, mX);
}

DynamicVector<double> operator*(const DynamicVector<double>& aX, const JacobianColumnEvaluator& aA)
{
    auto tResult = std::vector<double>(aA.mColumns);
    for (unsigned int i = 0; i < aA.mColumns; ++i)
    {
        tResult[i] = aX.dot(aA.column(i));
    }
    return DynamicVector<double>(std::move(tResult));
}

JacobianMultiplier to_jacobian_multiplier(JacobianColumnEvaluator aJacobianColumnEvaluator)
{
    return JacobianMultiplier{/*.mNumColumns=*/aJacobianColumnEvaluator.mColumns,
                              /*.mJacobianTimesVector=*/[tColumnEvaluator = std::move(aJacobianColumnEvaluator)](
                                                            const DynamicVector<double>& aX)
                              { return aX * tColumnEvaluator; }};
}
}  // namespace plato::linear_algebra
