#include <gtest/gtest.h>

#include "plato/criteria/library/VectorSubsetFunction.hpp"

namespace plato::criteria::library::unittest
{
TEST(VectorSubsetFunction, Evaluation)
{
    const auto tIndices = std::set<std::size_t>{1U, 3U};
    const auto tSubsetFunction = make_vector_subset_function(tIndices);

    const auto tInput = linear_algebra::DynamicVector<double>{0.0, 1.0, 2.0, 3.0};
    const auto tResult = tSubsetFunction.evaluate<core::evaluation::kFunction>(tInput);
    const auto tExpected = linear_algebra::DynamicVector<double>{1.0, 3.0};
    EXPECT_EQ(tResult, tExpected);
}

TEST(VectorSubsetFunction, Jacobian)
{
    const auto tIndices = std::set<std::size_t>{0U, 1U, 3U};
    const auto tSubsetFunction = make_vector_subset_function(tIndices);

    const auto tInput = linear_algebra::DynamicVector<double>{0.0, 0.0, 0.0, 0.0};
    const auto tRowVector = linear_algebra::DynamicVector<double>{10.0, 11.0, 12.0};
    const auto tResult = tRowVector * tSubsetFunction.evaluate<core::evaluation::kFirstDerivative>(tInput);
    EXPECT_EQ(tResult, tRowVector);
}

TEST(VectorSubsetFunction, AdjointJacobian)
{
    const auto tIndices = std::set<std::size_t>{2U};
    const auto tSubsetFunction = make_vector_subset_function(tIndices);

    const auto tInput = linear_algebra::DynamicVector<double>{0.0, 0.0, 0.0, 0.0};
    const auto tRowVector = linear_algebra::DynamicVector<double>{10.0, 11.0, 12.0, 13.0};
    const auto tResult =
        tRowVector *
        tSubsetFunction.evaluate<core::evaluation::kFirstDerivative, core::MatrixOrdering::kAdjoint>(tInput);
    const auto tExpected = linear_algebra::DynamicVector<double>{12.0};
    EXPECT_EQ(tResult, tExpected);
}

}  // namespace plato::criteria::library::unittest
