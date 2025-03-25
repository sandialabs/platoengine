#include <gtest/gtest.h>

#include "plato/criteria/library/CriterionTraits.hpp"

namespace plato::criteria::library::unittest
{
TEST(CriterionTraits, ToParallelization)
{
    {
        constexpr auto tIsParallelized = true;
        EXPECT_EQ(to_parallelization(tIsParallelized), Parallelization::kParallel);
    }
    {
        constexpr auto tIsParallelized = false;
        EXPECT_EQ(to_parallelization(tIsParallelized), Parallelization::kSerial);
    }
}

TEST(CriterionTraits, ToFunctionDimension)
{
    {
        constexpr auto tIsScalar = true;
        EXPECT_EQ(to_function_dimension(tIsScalar), FunctionDimension::kScalar);
    }
    {
        constexpr auto tIsScalar = false;
        EXPECT_EQ(to_function_dimension(tIsScalar), FunctionDimension::kVector);
    }
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

TEST(CriterionRegistration, NumberOfTraits)
{
    constexpr auto tExpectedNumberOfTraits = static_cast<std::size_t>(Parallelization::kNumberOfEnumerates) *
                                             static_cast<std::size_t>(FunctionDimension::kNumberOfEnumerates);
    EXPECT_EQ(tExpectedNumberOfTraits, number_of_traits());
}

}  // namespace plato::criteria::library::unittest
