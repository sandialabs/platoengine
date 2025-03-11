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

TEST(CriterionRegistration, FactoryIndex)
{
    EXPECT_EQ(trait_index(CriterionTraits{Parallelization::kParallel, FunctionDimension::kScalar}), 0U);
    EXPECT_EQ(trait_index(CriterionTraits{Parallelization::kSerial, FunctionDimension::kScalar}), 1U);
    EXPECT_EQ(trait_index(CriterionTraits{Parallelization::kParallel, FunctionDimension::kVector}), 2U);
    EXPECT_EQ(trait_index(CriterionTraits{Parallelization::kSerial, FunctionDimension::kVector}), 3U);

    EXPECT_EQ(trait_index(Parallelization::kParallel, FunctionDimension::kScalar), 0U);
    EXPECT_EQ(trait_index(Parallelization::kSerial, FunctionDimension::kScalar), 1U);
    EXPECT_EQ(trait_index(Parallelization::kParallel, FunctionDimension::kVector), 2U);
    EXPECT_EQ(trait_index(Parallelization::kSerial, FunctionDimension::kVector), 3U);
}

TEST(CriterionRegistration, FactoryTraitsFromIndex)
{
    EXPECT_EQ(traits_from_index(0U).mParallelization, Parallelization::kParallel);
    EXPECT_EQ(traits_from_index(0U).mDimension, FunctionDimension::kScalar);
    EXPECT_EQ(traits_from_index(1U).mParallelization, Parallelization::kSerial);
    EXPECT_EQ(traits_from_index(1U).mDimension, FunctionDimension::kScalar);

    EXPECT_EQ(traits_from_index(2U).mParallelization, Parallelization::kParallel);
    EXPECT_EQ(traits_from_index(2U).mDimension, FunctionDimension::kVector);
    EXPECT_EQ(traits_from_index(3U).mParallelization, Parallelization::kSerial);
    EXPECT_EQ(traits_from_index(3U).mDimension, FunctionDimension::kVector);
}

}  // namespace plato::criteria::library::unittest
