#include "plato/test_utilities/RandomPerturbationVector.hpp"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <random>

#include "plato/linear_algebra/DynamicVector.hpp"
#include "plato/utilities/ContainerHelpers.hpp"

namespace plato::test_utilities
{
auto random_perturbation_vector(const std::size_t aSize) -> linear_algebra::DynamicVector<double>
{
    assert(aSize != 0);
    auto tEngine = std::default_random_engine{std::random_device{}()};
    return random_perturbation_vector(aSize, tEngine);
}

auto random_perturbation_vector(const std::size_t aSize, std::default_random_engine& aRandomEngine)
    -> linear_algebra::DynamicVector<double>
{
    assert(aSize != 0);
    std::uniform_real_distribution<double> tDistribution{-1, 1};

    auto tRandomValues = utilities::reserved_container<std::vector<double>>(aSize);
    std::ranges::generate_n(std::back_inserter(tRandomValues), static_cast<unsigned int>(aSize),
                            [&aRandomEngine, &tDistribution]() { return tDistribution(aRandomEngine); });

    auto tRandomVector = linear_algebra::DynamicVector<double>(std::move(tRandomValues));
    const auto tNorm = std::sqrt(tRandomVector.dot(tRandomVector));
    tRandomVector *= 1. / tNorm;

    return tRandomVector;
}
}  // namespace plato::test_utilities
