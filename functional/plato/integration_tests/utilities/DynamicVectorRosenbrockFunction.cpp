#include "plato/integration_tests/utilities/DynamicVectorRosenbrockFunction.hpp"

#include "plato/core/Function.hpp"
#include "plato/linear_algebra/DynamicVector.hpp"
#include "plato/test_utilities/Rosenbrock.hpp"
#include "plato/test_utilities/TwoDTestTypes.hpp"
#include "plato/test_utilities/TwoDTestTypesToDynamicVector.hpp"

namespace plato::integration_tests::utilities
{
auto make_rosenbrock_dynamic_vector_function(const test_utilities::Rosenbrock& aRosenbrock)
    -> core::Function<double, linear_algebra::DynamicVector<double>, const linear_algebra::DynamicVector<double>&>
{
    return core::make_function(
        [rosenbrock = aRosenbrock](const linear_algebra::DynamicVector<double>& x)
        {
            assert(x.size() == 2);
            return rosenbrock.f(x[0], x[1]);
        },
        [rosenbrock = aRosenbrock](const linear_algebra::DynamicVector<double>& x)
        {
            assert(x.size() == 2);
            return to_dynamic_vector(rosenbrock.df(x[0], x[1]));
        });
}

}  // namespace plato::integration_tests::utilities
