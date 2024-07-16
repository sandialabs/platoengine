#ifndef PLATO_ROL_INTEGRATION_UNITTEST_DYNAMICVECTORTESTUTILITIES
#define PLATO_ROL_INTEGRATION_UNITTEST_DYNAMICVECTORTESTUTILITIES

#include "plato/core/Function.hpp"
#include "plato/linear_algebra/DynamicVector.hpp"
#include "plato/test_utilities/Himmelblau.hpp"
#include "plato/test_utilities/TwoDTestTypes.hpp"

namespace plato::third_party_integration::rol::unittest
{
[[nodiscard]] inline auto to_dynamic_vector(const test_utilities::TwoDVector& aX)
    -> linear_algebra::DynamicVector<double>
{
    return linear_algebra::DynamicVector<double>{std::vector{aX(0), aX(1)}};
}

[[nodiscard]] inline auto make_himmelblau_dynamic_vector_function(const test_utilities::Himmelblau& aHimmelblau)
{
    return core::make_function([himmelblau = aHimmelblau](const linear_algebra::DynamicVector<double>& x)
                               { return himmelblau.f(x[0], x[1]); },
                               [himmelblau = aHimmelblau](const linear_algebra::DynamicVector<double>& x)
                               { return to_dynamic_vector(himmelblau.df(x[0], x[1])); });
}

}  // namespace plato::third_party_integration::rol::unittest

#endif
