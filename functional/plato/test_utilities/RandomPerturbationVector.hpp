#ifndef PLATO_TEST_UTILITIES_RANDOMPERTURBATIONVECTOR
#define PLATO_TEST_UTILITIES_RANDOMPERTURBATIONVECTOR

#include <random>

#include "plato/linear_algebra/DynamicVector.hpp"

namespace plato::test_utilities
{
/// @brief Create a DynamicVector of size @a aSize whose components are random values between [-1,1]. Uses a random
/// engine seeded with std::random_device to generate values. The vector is then normalized.
auto random_perturbation_vector(const std::size_t aSize) -> linear_algebra::DynamicVector<double>;

/// @brief Create a DynamicVector of size @a aSize whose components are random values between [-1,1] generated with @a
/// aRandomEngine. The vector is then normalized.
auto random_perturbation_vector(const std::size_t aSize, std::default_random_engine& aRandomEngine)
    -> linear_algebra::DynamicVector<double>;
}  // namespace plato::test_utilities

#endif
