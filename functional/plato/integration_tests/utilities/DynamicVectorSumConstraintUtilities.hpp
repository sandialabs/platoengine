#ifndef PLATO_INTEGRATION_TESTS_UTILITIES_DYNAMICVECTORSUMCONSTRAINTUTILITIES
#define PLATO_INTEGRATION_TESTS_UTILITIES_DYNAMICVECTORSUMCONSTRAINTUTILITIES

#include "plato/core/Function.hpp"
#include "plato/linear_algebra/DynamicVector.hpp"
#include "plato/linear_algebra/JacobianMultiplier.hpp"

namespace plato::integration_tests::utilities
{

[[nodiscard]] auto make_line_and_circle_jacobian_function()
    -> core::Function<linear_algebra::DynamicVector<double>,
                      linear_algebra::JacobianMultiplier,
                      const linear_algebra::DynamicVector<double>&>;

[[nodiscard]] auto make_line_and_circle_adjoint_jacobian_function()
    -> core::Function<linear_algebra::DynamicVector<double>,
                      linear_algebra::JacobianMultiplier,
                      const linear_algebra::DynamicVector<double>&>;
}  // namespace plato::integration_tests::utilities

#endif
