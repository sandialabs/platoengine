#ifndef PLATO_LINEAR_ALGEBRA_JACOBIANMULTIPLIER
#define PLATO_LINEAR_ALGEBRA_JACOBIANMULTIPLIER

#include <functional>

#include "plato/linear_algebra/DynamicVector.hpp"

namespace plato::linear_algebra
{
/// @brief An object representing the multiplication of a row vector and a Jacobian matrix.
struct JacobianMultiplier
{
    using JacobianTimesVectorFunction = std::function<DynamicVector<double>(const DynamicVector<double>&)>;

    JacobianTimesVectorFunction mJacobianTimesVectorFunction;
};

/// @brief Implementation of multiplication of a row vector @a aX
template <typename Arg>
[[nodiscard]] auto operator*(const Arg& aX, const JacobianMultiplier& aA)
    -> std::enable_if_t<!std::is_same_v<std::decay_t<Arg>, JacobianMultiplier>, DynamicVector<double>>
{
    return aA.mJacobianTimesVectorFunction(aX);
}

/// @brief Implementation of multiplication of two JacobianMultipliers
[[nodiscard]] inline auto operator*(JacobianMultiplier aF, JacobianMultiplier aG) -> JacobianMultiplier
{
    return JacobianMultiplier{[tF = std::move(aF), tG = std::move(aG)](const DynamicVector<double>& aDirection)
                              { return (aDirection * tF) * tG; }};
}

}  // namespace plato::linear_algebra

#endif
