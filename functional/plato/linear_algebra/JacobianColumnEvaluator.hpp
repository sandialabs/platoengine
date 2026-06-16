#ifndef PLATO_LINEAR_ALGEBRA_JACOBIANCOLUMNEVALUATOR
#define PLATO_LINEAR_ALGEBRA_JACOBIANCOLUMNEVALUATOR

#include <functional>

#include "plato/linear_algebra/DynamicVector.hpp"

namespace plato::linear_algebra
{
struct JacobianMultiplier;

/// @brief An implementation of a Jacobian that generates one column at a time.
///
/// @note This is mainly for testing, prefer JacobianMultiplier.
/// @sa JacobianMultiplier
struct JacobianColumnEvaluator
{
    /// @return Column with index @a aIndex.
    [[nodiscard]] auto column(unsigned int aIndex) const -> DynamicVector<double>;

    using ColumnFunction = std::function<DynamicVector<double>(unsigned int, const DynamicVector<double>&)>;

    unsigned int mColumns = 0;
    DynamicVector<double> mX;
    ColumnFunction mColumnFunction;
};

/// Implements multiplication of a row vector @a aX with the Jacobian matrix represented by @a aA
[[nodiscard]] auto operator*(const DynamicVector<double>& aRowVector, const JacobianColumnEvaluator& aJacobian)
    -> DynamicVector<double>;

/// @brief Creates a JacobianMultiplier from @a aJacobianColumnEvaluator
auto to_jacobian_multiplier(JacobianColumnEvaluator aJacobianColumnEvaluator) -> JacobianMultiplier;

}  // namespace plato::linear_algebra

#endif
