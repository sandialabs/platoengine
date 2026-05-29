#ifndef PLATO_CORE_TEST_UTILITIES_UTILITIES
#define PLATO_CORE_TEST_UTILITIES_UTILITIES

#include "plato/core/Function.hpp"
#include "plato/test_utilities/Penalty.hpp"
#include "plato/test_utilities/Rosenbrock.hpp"
#include "plato/test_utilities/TwoDTestTypes.hpp"

namespace plato::core::test_utilities
{
/// @brief Creates a core::Function from an object with members `f` and `df`.
/// @tparam FunctionObject A class or struct with members `f` and `df`, which both have two `double` arguments.
template <typename FunctionObject>
[[nodiscard]] auto make_two_d_function(const FunctionObject& aFunctionObject)
{
    namespace pftu = plato::test_utilities;
    return make_function_with_first_derivative(
        [aFunctionObject](const pftu::TwoDVector& aVector) { return aFunctionObject.f(aVector(0), aVector(1)); },
        [aFunctionObject](const pftu::TwoDVector& aVector) { return aFunctionObject.df(aVector(0), aVector(1)); });
}

}  // namespace plato::core::test_utilities

#endif
