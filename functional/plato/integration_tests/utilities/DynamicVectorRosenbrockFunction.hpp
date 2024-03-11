#ifndef PLATO_INTEGRATION_TESTS_UTILITIES_DYNAMICVECTORTESTUTILITIES
#define PLATO_INTEGRATION_TESTS_UTILITIES_DYNAMICVECTORTESTUTILITIES

#include "plato/core/Function.hpp"
#include "plato/linear_algebra/DynamicVector.hpp"
#include "plato/test_utilities/Rosenbrock.hpp"
#include "plato/test_utilities/TwoDTestTypes.hpp"

namespace plato::integration_tests::utilities
{
[[nodiscard]] inline auto to_dynamic_vector(const test_utilities::TwoDVector& aX)
    -> linear_algebra::DynamicVector<double>
{
    return linear_algebra::DynamicVector<double>{std::vector{aX(0), aX(1)}};
}

[[nodiscard]] inline auto make_rosenbrock_dynamic_vector_function(const test_utilities::Rosenbrock& aRosenbrock)
{
    return core::make_function([rosenbrock = aRosenbrock](const linear_algebra::DynamicVector<double>& x)
                               { return rosenbrock.f(x[0], x[1]); },
                               [rosenbrock = aRosenbrock](const linear_algebra::DynamicVector<double>& x)
                               { return to_dynamic_vector(rosenbrock.df(x[0], x[1])); });
}

}  // namespace plato::integration_tests::utilities

#endif
