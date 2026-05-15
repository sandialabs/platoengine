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

auto operator*(const DynamicVector<double>& aX, const JacobianColumnEvaluator& aA) -> DynamicVector<double>
{
    auto tResult = std::vector<double>(aA.mColumns);
    for (unsigned int i = 0; i < aA.mColumns; ++i)
    {
        tResult[i] = aX.dot(aA.column(i));
    }
    return DynamicVector<double>(std::move(tResult));
}

auto to_jacobian_multiplier(JacobianColumnEvaluator aJacobianColumnEvaluator) -> JacobianMultiplier
{
    return JacobianMultiplier{/*.mJacobianTimesVector=*/
                              [tColumnEvaluator = std::move(aJacobianColumnEvaluator)](const DynamicVector<double>& aX)
                              { return aX * tColumnEvaluator; }};
}
}  // namespace plato::linear_algebra
