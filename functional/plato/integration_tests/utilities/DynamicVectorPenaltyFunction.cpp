#include "plato/integration_tests/utilities/DynamicVectorPenaltyFunction.hpp"

#include <cassert>

#include "plato/integration_tests/utilities/DynamicVectorRosenbrockFunction.hpp"

namespace plato::integration_tests::utilities
{
linear_algebra::DynamicVector<double> DynamicVectorJacobian::column(const int aIndex) const
{
    return linear_algebra::DynamicVector<double>{mJacobian(0, aIndex), mJacobian(1, aIndex)};
}

linear_algebra::DynamicVector<double> operator*(const linear_algebra::DynamicVector<double>& aX,
                                                const DynamicVectorJacobian& aJacobian)
{
    assert(aX.size() == 2);
    return linear_algebra::DynamicVector<double>{aX.dot(aJacobian.column(0)), aX.dot(aJacobian.column(1))};
}

auto make_penalty_dynamic_vector_function(const test_utilities::Penalty& aPenalty) -> core::
    Function<linear_algebra::DynamicVector<double>, DynamicVectorJacobian, const linear_algebra::DynamicVector<double>&>
{
    return core::make_function([p = aPenalty](const linear_algebra::DynamicVector<double>& x)
                               { return to_dynamic_vector(p.f(x[0], x[1])); },
                               [p = aPenalty](const linear_algebra::DynamicVector<double>& x)
                               { return DynamicVectorJacobian{p.df(x[0], x[1])}; });
}

}  // namespace plato::integration_tests::utilities