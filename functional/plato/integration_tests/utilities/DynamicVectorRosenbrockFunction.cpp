#include "plato/integration_tests/utilities/DynamicVectorRosenbrockFunction.hpp"

#include <cassert>

#include "plato/core/Function.hpp"
#include "plato/core/FunctionHelpers.hpp"
#include "plato/linear_algebra/DynamicVector.hpp"
#include "plato/test_utilities/Rosenbrock.hpp"
#include "plato/test_utilities/TwoDTestTypesToDynamicVector.hpp"

namespace plato::integration_tests::utilities
{
auto make_rosenbrock_dynamic_vector_function(const test_utilities::Rosenbrock& aRosenbrock)
    -> core::Function<const linear_algebra::DynamicVector<double>&,
                      core::FunctionInfo<double, core::evaluation::kFunction>,
                      core::FunctionInfo<linear_algebra::DynamicVector<double>, core::evaluation::kFirstDerivative>>
{
    return core::make_function_with_first_derivative(
        [tRosenbrock = aRosenbrock](const linear_algebra::DynamicVector<double>& aVector)
        {
            assert(aVector.size() == 2);
            return tRosenbrock.f(aVector[0], aVector[1]);
        },
        [tRosenbrock = aRosenbrock](const linear_algebra::DynamicVector<double>& aVector)
        {
            assert(aVector.size() == 2);
            return to_dynamic_vector(tRosenbrock.df(aVector[0], aVector[1]));
        });
}

}  // namespace plato::integration_tests::utilities
