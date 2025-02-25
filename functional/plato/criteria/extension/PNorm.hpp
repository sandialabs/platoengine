#ifndef PLATO_CRITERIA_EXTENSION_PNORM
#define PLATO_CRITERIA_EXTENSION_PNORM

#include <vector>

#include "plato/core/Function.hpp"
#include "plato/linear_algebra/DynamicVector.hpp"

namespace plato::criteria::extension
{

using PNormFunction =
    core::Function<const linear_algebra::DynamicVector<double>&,
                   core::FunctionInfo<double, core::evaluation::kFunction>,
                   core::FunctionInfo<linear_algebra::DynamicVector<double>, core::evaluation::kFirstDerivative>>;

/// @brief Returns a PNormFunction (core::Function) that applies a p-norm with power @a aPower to the vector criteria
/// @pre @a aPower >= 1
[[nodiscard]] auto make_p_norm_function(const double aPower) -> PNormFunction;

namespace detail
{

/// @brief Returns the value of the p-norm applied to a vector @a aVector with power with power @a aPower
/// @pre @a aVector is not empty and @a Power >= 1
[[nodiscard]] double p_norm_value(std::vector<double> aVector, const double aPower);

/// @brief Returns a vector containing the gradient of the p-norm applied to a vector @a aVector with power with power
/// @a aPower
/// @pre @a aVector is not empty and @a aPower >= 1
[[nodiscard]] auto p_norm_gradient(std::vector<double> aVector, const double aPower) -> std::vector<double>;

/// @brief Evaluates the inside of the p-norm expression applied to a vector @a aVector with power with power
/// @a aPower before the inverse power is applied so that it can be used in both the value function and the gradient
/// @pre @a aVector is not empty and @a aPower >= 1
[[nodiscard]] auto p_norm_expression(std::vector<double> aVector, const double aPower) -> double;

}  // namespace detail

}  // namespace plato::criteria::extension

#endif
