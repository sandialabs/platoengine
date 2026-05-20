#ifndef PLATO_INTEGRATION_TESTS_UTILITIES_DYNAMICVECTORPENALTYFUNCTION
#define PLATO_INTEGRATION_TESTS_UTILITIES_DYNAMICVECTORPENALTYFUNCTION

#include "plato/core/Function.hpp"
#include "plato/linear_algebra/DynamicVector.hpp"
#include "plato/test_utilities/Penalty.hpp"

namespace plato::integration_tests::utilities
{
struct DynamicVectorJacobian
{
    [[nodiscard]] auto column(int aIndex) const -> linear_algebra::DynamicVector<double>;
    test_utilities::TwoDMatrix mJacobian;
};

[[nodiscard]] auto operator*(const linear_algebra::DynamicVector<double>& aRowVector,
                             const DynamicVectorJacobian& aJacobian) -> linear_algebra::DynamicVector<double>;

[[nodiscard]] auto make_penalty_dynamic_vector_function(const test_utilities::Penalty& aPenalty)
    -> core::Function<const linear_algebra::DynamicVector<double>&,
                      core::FunctionInfo<linear_algebra::DynamicVector<double>, core::evaluation::kFunction>,
                      core::FunctionInfo<DynamicVectorJacobian, core::evaluation::kFirstDerivative>>;

}  // namespace plato::integration_tests::utilities

#endif
