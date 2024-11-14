#ifndef PLATO_INTEGRATION_TESTS_UTILITIES_DYNAMICVECTORSUMCONSTRAINTUTILITIES
#define PLATO_INTEGRATION_TESTS_UTILITIES_DYNAMICVECTORSUMCONSTRAINTUTILITIES

#include "plato/core/Function.hpp"
#include "plato/linear_algebra/DynamicVector.hpp"
#include "plato/linear_algebra/JacobianMultiplier.hpp"

namespace plato::integration_tests::utilities
{
using LineAndCircleFunction =
    core::Function<const linear_algebra::DynamicVector<double>&,
                   core::FunctionInfo<linear_algebra::DynamicVector<double>, core::evaluation::kFunction>,
                   core::FunctionInfo<linear_algebra::JacobianMultiplier, core::evaluation::kFirstDerivative>,
                   core::FunctionInfo<linear_algebra::AdjointJacobianMultiplier,
                                      core::evaluation::kFirstDerivative,
                                      core::MatrixOrdering::kAdjoint> >;

/// @brief Creates a 2D vector function of two arguments for testing.
///
/// The function has two components and two arguments. The first component is \f$x + y\f$ and the second is \f$x^2 +
/// y^2\f$
[[nodiscard]] auto make_line_and_circle_jacobian_function() -> LineAndCircleFunction;
}  // namespace plato::integration_tests::utilities

#endif
