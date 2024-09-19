#ifndef PLATO_LINEAR_ALGEBRA_JACOBIANMULTIPLIER
#define PLATO_LINEAR_ALGEBRA_JACOBIANMULTIPLIER

#include <iostream>

#include "plato/linear_algebra/DynamicVector.hpp"

namespace plato::linear_algebra
{

/// @brief An object representing the multiplication of a row vector and a Jacobian matrix.
struct JacobianMultiplier
{
    using JacobianTimesVectorFunction = std::function<DynamicVector<double>(const DynamicVector<double>&)>;

    JacobianTimesVectorFunction mJacobianTimesVectorFunction;

    /// @brief Implementation of multiplication of two JacobianMultipliers
    JacobianMultiplier operator*(const JacobianMultiplier& aG) const
    {
        auto tComposedFunction = [tF = std::move(*this), tG = std::move(aG)](const DynamicVector<double>& aDirection)
        { return tF.mJacobianTimesVectorFunction(tG.mJacobianTimesVectorFunction(aDirection)); };

        return JacobianMultiplier{tComposedFunction};
    }
};

/// @brief Implementation of multiplication of a row vector @a aX
template <typename Arg>
[[nodiscard]] DynamicVector<double> operator*(const Arg& aX, const JacobianMultiplier& aA)
{
    return aA.mJacobianTimesVectorFunction(aX);
}

}  // namespace plato::linear_algebra

#endif
