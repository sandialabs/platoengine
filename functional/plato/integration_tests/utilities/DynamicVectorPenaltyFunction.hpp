#ifndef PLATO_INTEGRATION_TESTS_UTILITIES_DYNAMICVECTORPENALTYFUNCTION
#define PLATO_INTEGRATION_TESTS_UTILITIES_DYNAMICVECTORPENALTYFUNCTION

#include "plato/core/Function.hpp"
#include "plato/linear_algebra/DynamicVector.hpp"
#include "plato/test_utilities/Penalty.hpp"

namespace plato::integration_tests::utilities
{
struct DynamicVectorJacobian
{
    linear_algebra::DynamicVector<double> column(const int aIndex) const;
    test_utilities::TwoDMatrix mJacobian;
};

[[nodiscard]] linear_algebra::DynamicVector<double> operator*(const linear_algebra::DynamicVector<double>& aX,
                                                              const DynamicVectorJacobian& aJacobian);

[[nodiscard]] auto make_penalty_dynamic_vector_function(const test_utilities::Penalty& aPenalty)
    -> core::Function<linear_algebra::DynamicVector<double>,
                      DynamicVectorJacobian,
                      const linear_algebra::DynamicVector<double>&>;

}  // namespace plato::integration_tests::utilities

#endif
