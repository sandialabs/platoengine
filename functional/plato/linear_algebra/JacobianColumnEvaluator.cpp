#include "plato/linear_algebra/JacobianColumnEvaluator.hpp"

#include <utility>
#include <vector>

#include "plato/linear_algebra/DynamicVector.hpp"
#include "plato/linear_algebra/JacobianMultiplier.hpp"

namespace plato::linear_algebra
{
auto JacobianColumnEvaluator::column(const unsigned int aIndex) const -> DynamicVector<double>
{
    return mColumnFunction(aIndex, mX);
}

auto operator*(const DynamicVector<double>& aRowVector, const JacobianColumnEvaluator& aJacobian)
    -> DynamicVector<double>
{
    auto tResult = std::vector<double>(aJacobian.mColumns);
    for (unsigned int tColumnIndex = 0; tColumnIndex < aJacobian.mColumns; ++tColumnIndex)
    {
        tResult[tColumnIndex] = aRowVector.dot(aJacobian.column(tColumnIndex));
    }
    return DynamicVector<double>(std::move(tResult));
}

auto to_jacobian_multiplier(JacobianColumnEvaluator aJacobianColumnEvaluator) -> JacobianMultiplier
{
    return JacobianMultiplier{.mVectorTimesJacobianFunction = [tColumnEvaluator = std::move(aJacobianColumnEvaluator)](
                                                                  const DynamicVector<double>& aRowVector)
                              { return aRowVector * tColumnEvaluator; }};
}
}  // namespace plato::linear_algebra
