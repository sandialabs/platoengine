#ifndef PLATO_TEST_UTILITIES_TWODTESTTYPESTODYNAMICVECTOR
#define PLATO_TEST_UTILITIES_TWODTESTTYPESTODYNAMICVECTOR

#include "plato/linear_algebra/DynamicVector.hpp"
#include "plato/test_utilities/TwoDTestTypes.hpp"

namespace plato::test_utilities
{
/// @brief Converts @a aX to a DynamicVector
[[nodiscard]] auto to_dynamic_vector(const TwoDVector& aX) -> linear_algebra::DynamicVector<double>;

}  // namespace plato::test_utilities

#endif
