#ifndef PLATO_LINEAR_ALGEBRA_COMPOSEADJOINTJACOBIANMULTIPLIER
#define PLATO_LINEAR_ALGEBRA_COMPOSEADJOINTJACOBIANMULTIPLIER

#include <type_traits>
#include <utility>

#include "plato/core/Function.hpp"
#include "plato/linear_algebra/DynamicVector.hpp"
#include "plato/linear_algebra/JacobianMultiplier.hpp"

namespace plato::linear_algebra
{
/// @brief Generates a new Function that is the composition of @a f and @a g, i.e. \f$f(g(x))\f$
template <typename fArg, typename gR, typename gArg>
[[nodiscard]] auto compose_adjoint_jacobian(core::Function<DynamicVector<double>, JacobianMultiplier, fArg> aF,
                                            core::Function<gR, JacobianMultiplier, gArg> aG)
{
    static_assert(std::is_convertible_v<gR, fArg>,
                  "The return type of aG must be convertible to the argument type of aF.");

    auto tFOfG = [aF, aG](const gArg& aX) { return aF.f(aG.f(aX)); };

    auto tDfOfGTranspose = [tF = std::move(aF), tG = std::move(aG)](const gArg& tX)
    { return tG.df(tX) * tF.df(tG.f(tX)); };

    return core::Function<DynamicVector<double>, JacobianMultiplier, gArg>{std::move(tFOfG),
                                                                           std::move(tDfOfGTranspose)};
}

}  // namespace plato::linear_algebra
#endif
