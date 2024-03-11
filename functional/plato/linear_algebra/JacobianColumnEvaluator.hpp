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
    [[nodiscard]] DynamicVector<double> column(const unsigned int aIndex) const;

    using ColumnFunction = std::function<DynamicVector<double>(unsigned int, const DynamicVector<double>&)>;

    unsigned int mColumns = 0;
    DynamicVector<double> mX;
    ColumnFunction mColumnFunction;
};

/// Implements multiplication of a row vector @a aX with the Jacobian matrix represented by @a aA
[[nodiscard]] DynamicVector<double> operator*(const DynamicVector<double>& aX, const JacobianColumnEvaluator& aA);

/// @brief Creates a JacobianMultiplier from @a aJacobianColumnEvaluator
JacobianMultiplier to_jacobian_multiplier(JacobianColumnEvaluator aJacobianColumnEvaluator);

}  // namespace plato::linear_algebra

#endif
