#include "plato/test_utilities/TwoDTestTypesToDynamicVector.hpp"

namespace plato::test_utilities
{
auto to_dynamic_vector(const TwoDVector& aX) -> linear_algebra::DynamicVector<double>
{
    return linear_algebra::DynamicVector<double>{std::vector{aX(0), aX(1)}};
}

}  // namespace plato::test_utilities
