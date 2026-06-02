#include <gtest/gtest.h>

#include <cmath>

#include "plato/linear_algebra/RandomPerturbationVector.hpp"

namespace plato::linear_algebra::unittest
{
TEST(RandomPerturbationVector, IsNormalVector)
{
    const auto tRandomPerturbation = random_perturbation_vector(6);

    for (const auto& tValue : tRandomPerturbation.stdVector())
    {
        EXPECT_LE(tValue, 1.0);
        EXPECT_GE(tValue, -1.0);
    }

    constexpr double tTolerance{1.0e-12};
    EXPECT_NEAR(std::sqrt(tRandomPerturbation.dot(tRandomPerturbation)), 1.0, tTolerance);
}

}  // namespace plato::linear_algebra::unittest
