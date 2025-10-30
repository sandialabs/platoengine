#include "plato/linear_algebra/JacobianMultiplier.hpp"

namespace plato::linear_algebra
{
[[nodiscard]] auto make_adjoint_jacobian_multiplier(JacobianMultiplier::VectorTimesJacobianFunction aProductFunction)
    -> AdjointJacobianMultiplier
{
    return AdjointJacobianMultiplier{JacobianMultiplier{std::move(aProductFunction)}};
}
}  // namespace plato::linear_algebra
