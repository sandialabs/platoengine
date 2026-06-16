#ifndef PLATO_LINEAR_ALGEBRA_JACOBIANMULTIPLIER
#define PLATO_LINEAR_ALGEBRA_JACOBIANMULTIPLIER

#include <functional>

#include "plato/linear_algebra/DynamicVector.hpp"
#include "plato/utilities/NamedType.hpp"

namespace plato::linear_algebra
{
/// @brief An object representing the multiplication of a row vector and a Jacobian matrix.
struct JacobianMultiplier
{
    using VectorTimesJacobianFunction = std::function<DynamicVector<double>(const DynamicVector<double>&)>;

    VectorTimesJacobianFunction mVectorTimesJacobianFunction;
};

/// @brief A NamedType to distinguish adjoint matrix-vector multiplication.
using AdjointJacobianMultiplier = utilities::NamedType<JacobianMultiplier, struct AdjointJacobianMultiplierTag>;

/// @brief Returns an AdjointJacobianMultiplier implemented with the product function @a aProductFunction
[[nodiscard]] auto make_adjoint_jacobian_multiplier(JacobianMultiplier::VectorTimesJacobianFunction aProductFunction)
    -> AdjointJacobianMultiplier;

/// @brief Implementation of multiplication of a row vector @a aRowVector
template <typename Arg>
[[nodiscard]] auto operator*(const Arg& aRowVector, const JacobianMultiplier& aJacobian)
    -> std::enable_if_t<!std::is_same_v<std::decay_t<Arg>, JacobianMultiplier>, DynamicVector<double>>;

/// @brief Implementation of multiplication of two JacobianMultipliers
[[nodiscard]] inline auto operator*(JacobianMultiplier aJacobianLeft, JacobianMultiplier aJacobianRight)
    -> JacobianMultiplier;

/// @brief Implementation of multiplication with an AdjointJacobianMultiplier
template <typename Arg>
auto operator*(const Arg& aRowVector, const AdjointJacobianMultiplier& aJacobian)
    -> std::enable_if_t<!std::is_same_v<std::decay_t<Arg>, AdjointJacobianMultiplier>, DynamicVector<double>>;

/// @brief Implementation of multiplication of two AdjointJacobianMultipliers
inline auto operator*(AdjointJacobianMultiplier aAdjointLeft, AdjointJacobianMultiplier aAdjointRight)
    -> AdjointJacobianMultiplier;

template <typename Arg>
auto operator*(const Arg& aRowVector, const JacobianMultiplier& aJacobian)
    -> std::enable_if_t<!std::is_same_v<std::decay_t<Arg>, JacobianMultiplier>, DynamicVector<double>>
{
    return aJacobian.mVectorTimesJacobianFunction(aRowVector);
}

inline auto operator*(JacobianMultiplier aJacobianLeft, JacobianMultiplier aJacobianRight) -> JacobianMultiplier
{
    return JacobianMultiplier{[tJacobianLeft = std::move(aJacobianLeft),
                               tJacobianRight = std::move(aJacobianRight)](const DynamicVector<double>& aDirection)
                              { return (aDirection * tJacobianLeft) * tJacobianRight; }};
}

template <typename Arg>
auto operator*(const Arg& aRowVector, const AdjointJacobianMultiplier& aJacobian)
    -> std::enable_if_t<!std::is_same_v<std::decay_t<Arg>, AdjointJacobianMultiplier>, DynamicVector<double>>
{
    return aRowVector * aJacobian.mValue;
}

inline auto operator*(AdjointJacobianMultiplier aAdjointLeft, AdjointJacobianMultiplier aAdjointRight)
    -> AdjointJacobianMultiplier
{
    return AdjointJacobianMultiplier{std::move(aAdjointLeft).mValue * std::move(aAdjointRight).mValue};
}

}  // namespace plato::linear_algebra

#endif
