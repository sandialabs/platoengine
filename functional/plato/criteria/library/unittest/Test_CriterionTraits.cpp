#include <gtest/gtest.h>

#include "plato/criteria/library/CriterionTraits.hpp"

namespace plato::criteria::library::unittest
{
TEST(CriterionTraits, ToParallelization)
{
    constexpr auto tParallelized = true;
    EXPECT_EQ(to_parallelization(tParallelized), Parallelization::kParallel);

    constexpr auto tSerial = false;
    EXPECT_EQ(to_parallelization(tSerial), Parallelization::kSerial);
}

TEST(CriterionTraits, ToFunctionDimension)
{
    constexpr auto tScalar = true;
    EXPECT_EQ(to_function_dimension(tScalar), FunctionDimension::kScalar);

    constexpr auto tVector = false;
    EXPECT_EQ(to_function_dimension(tVector), FunctionDimension::kVector);
}
}  // namespace plato::criteria::library::unittest
