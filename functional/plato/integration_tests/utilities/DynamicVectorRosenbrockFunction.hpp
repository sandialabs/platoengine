#ifndef PLATO_INTEGRATION_TESTS_UTILITIES_DYNAMICVECTORROSENBROCKFUNCTION
#define PLATO_INTEGRATION_TESTS_UTILITIES_DYNAMICVECTORROSENBROCKFUNCTION

#include "plato/core/Function.hpp"
#include "plato/linear_algebra/DynamicVector.hpp"
#include "plato/test_utilities/Rosenbrock.hpp"

namespace plato::test_utilities
{
class Rosenbrock;
}

namespace plato::integration_tests::utilities
{
/// @brief Creates a Function using Rosenbrock, with an interface that uses DynamicVector
[[nodiscard]] auto make_rosenbrock_dynamic_vector_function(const test_utilities::Rosenbrock& aRosenbrock)
    -> core::Function<double, linear_algebra::DynamicVector<double>, const linear_algebra::DynamicVector<double>&>;
}  // namespace plato::integration_tests::utilities

#endif
