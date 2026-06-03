#include "plato/test_utilities/TwoDTestTypesToDynamicVector.hpp"

#include <vector>

#include "plato/linear_algebra/DynamicVector.hpp"
#include "plato/test_utilities/TwoDTestTypes.hpp"

namespace plato::test_utilities
{
auto to_dynamic_vector(const TwoDVector& aTwoDVector) -> linear_algebra::DynamicVector<double>
{
    return linear_algebra::DynamicVector<double>{std::vector{aTwoDVector(0), aTwoDVector(1)}};
}

}  // namespace plato::test_utilities
