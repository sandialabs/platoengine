#ifndef PLATO_CRITERIA_LIBRARY_TARGETOFFSETFUNCTION
#define PLATO_CRITERIA_LIBRARY_TARGETOFFSETFUNCTION

#include "plato/core/Function.hpp"
#include "plato/linear_algebra/DynamicVector.hpp"
#include "plato/linear_algebra/JacobianMultiplier.hpp"

namespace plato::criteria::library
{
using TargetOffsetFunction =
    core::Function<const linear_algebra::DynamicVector<double>&,
                   core::FunctionInfo<linear_algebra::DynamicVector<double>, core::evaluation::kFunction>,
                   core::FunctionInfo<linear_algebra::JacobianMultiplier, core::evaluation::kFirstDerivative>,
                   core::FunctionInfo<linear_algebra::AdjointJacobianMultiplier,
                                      core::evaluation::kFirstDerivative,
                                      core::MatrixOrdering::kAdjoint>>;

/// @brief Creates a vector function that subtracts a vector of target values from a vector.
[[nodiscard]] auto make_target_offset_function(std::vector<double> aTargets) -> TargetOffsetFunction;

/// @brief Creates a vector function that subtracts a single scalar value from a vector.
[[nodiscard]] auto make_target_offset_function(double aTarget) -> TargetOffsetFunction;

}  // namespace plato::criteria::library

#endif
