#include "plato/integration_tests/utilities/DynamicVectorPenaltyFunction.hpp"

#include <cassert>

#include "plato/core/Function.hpp"
#include "plato/core/FunctionHelpers.hpp"
#include "plato/linear_algebra/DynamicVector.hpp"
#include "plato/test_utilities/Penalty.hpp"
#include "plato/test_utilities/TwoDTestTypesToDynamicVector.hpp"

namespace plato::integration_tests::utilities
{
auto DynamicVectorJacobian::column(const int aIndex) const -> linear_algebra::DynamicVector<double>
{
    return linear_algebra::DynamicVector<double>{mJacobian(0, aIndex), mJacobian(1, aIndex)};
}

auto operator*(const linear_algebra::DynamicVector<double>& aRowVector,
               const DynamicVectorJacobian& aJacobian) -> linear_algebra::DynamicVector<double>
{
    assert(aRowVector.size() == 2U);
    return linear_algebra::DynamicVector<double>{aRowVector.dot(aJacobian.column(0)),
                                                 aRowVector.dot(aJacobian.column(1))};
}

auto make_penalty_dynamic_vector_function(const test_utilities::Penalty& aPenalty)
    -> core::Function<const linear_algebra::DynamicVector<double>&,
                      core::FunctionInfo<linear_algebra::DynamicVector<double>, core::evaluation::kFunction>,
                      core::FunctionInfo<DynamicVectorJacobian, core::evaluation::kFirstDerivative>>
{
    return core::make_function_with_first_derivative(
        [tPenalty = aPenalty](const linear_algebra::DynamicVector<double>& aVector)
        { return to_dynamic_vector(tPenalty.f(aVector[0], aVector[1])); },
        [tPenalty = aPenalty](const linear_algebra::DynamicVector<double>& aVector)
        { return DynamicVectorJacobian{tPenalty.df(aVector[0], aVector[1])}; });
}

}  // namespace plato::integration_tests::utilities
