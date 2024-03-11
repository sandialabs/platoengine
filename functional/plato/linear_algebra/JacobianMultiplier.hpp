#ifndef PLATO_LINEAR_ALGEBRA_JACOBIANMULTIPLIER
#define PLATO_LINEAR_ALGEBRA_JACOBIANMULTIPLIER

#include "plato/linear_algebra/DynamicVector.hpp"

namespace plato::linear_algebra
{
/// @brief An object representing the multiplication of a row vector and a Jacobian matrix.
struct JacobianMultiplier
{
    using JacobianTimesVectorFunction = std::function<DynamicVector<double>(const DynamicVector<double>&)>;

    unsigned int mNumColumns = 0;
    JacobianTimesVectorFunction mJacobianTimesVectorFunction;
};

/// @brief Implementation of multiplication of a row vector @a aX
template <typename Arg>
[[nodiscard]] DynamicVector<double> operator*(const Arg& aX, const JacobianMultiplier& aA)
{
    return aA.mJacobianTimesVectorFunction(aX);
}

}  // namespace plato::linear_algebra

#endif
