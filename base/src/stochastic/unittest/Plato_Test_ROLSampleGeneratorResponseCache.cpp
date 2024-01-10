#include <gtest/gtest.h>

#include "Plato_DistributedVectorROL.hpp"
#include "Plato_ROLSampleGeneratorResponseCache.hpp"
#include "Plato_StochasticSample.hpp"

#include "ROL_BatchManager.hpp"
#include "ROL_MonteCarloGenerator.hpp"
#include "ROL_Uniform.hpp"

#include <algorithm>

namespace {

constexpr int kNumSamples = 10;

ROL::Ptr<ROL::SampleGenerator<double>> sample_generator()
{
    ROL::Ptr<ROL::BatchManager<double>> tBatchManager = ROL::makePtr<ROL::BatchManager<double>>();
    std::vector<ROL::Ptr<ROL::Distribution<double>>> tSampleDistributions{ROL::makePtr<ROL::Uniform<double>>()};
    constexpr bool tUseSA = false;
    constexpr bool tAdaptive = false;
    constexpr int tNumNewSamples = 0;
    constexpr int tRandomSeed = 42;
    return ROL::makePtr<ROL::MonteCarloGenerator<double>>(
        kNumSamples, tSampleDistributions, tBatchManager, tUseSA, tAdaptive, tNumNewSamples, tRandomSeed);
}
}

TEST(ROLSampleGeneratorCache, CachedValues)
{
    const auto tSampleGenerator = sample_generator();

    constexpr double tFunctionValue = 42.0;
    const auto f = [tFunctionValue](const ROL::Vector<double>&, const std::vector<Plato::Sample>& aSamples) -> std::vector<double>
    {
        std::vector<double> tResult(aSamples.size());
        std::fill(tResult.begin(), tResult.end(), tFunctionValue);
        return tResult;
    };

    Plato::ROLSampleGeneratorResponseCache<double> tCache{f, tSampleGenerator};
    constexpr std::size_t tNumElements = 11;
    constexpr double tInitialValue = 0.0;
    Plato::DistributedVectorROL tTestVector{MPI_COMM_WORLD, tNumElements, tInitialValue};
    const std::vector<Plato::Sample> tSamples = tCache.allSamples();

    EXPECT_EQ(tSamples.size(), static_cast<std::size_t>(kNumSamples));
    for(const auto& tSample : tSamples)
    {
        EXPECT_EQ(tCache(tTestVector, tSample), tFunctionValue);
        EXPECT_TRUE(tCache.exists(tTestVector, tSample));
    }
    EXPECT_TRUE(tCache.exists(tTestVector));
}

TEST(ROLSampleGeneratorCache, CachedValuesCalledTwice)
{
    // Tests that calling ROLSampleGeneratorResponseCache with a second
    // vector works correctly.
    const auto tSampleGenerator = sample_generator();

    const auto f = [](const ROL::Vector<double>&, const std::vector<Plato::Sample>& aSamples) -> std::vector<Plato::Sample>
    {
        return aSamples;
    };

    Plato::ROLSampleGeneratorResponseCache<Plato::Sample> tCache{f, tSampleGenerator};

    constexpr std::size_t tNumElements = 11;
    const Plato::DistributedVectorROL tTestVector1{MPI_COMM_WORLD, tNumElements, 0.0};

    const std::vector<Plato::Sample> tSamples = tCache.allSamples();
    EXPECT_EQ(tSamples.size(), static_cast<std::size_t>(kNumSamples));

    for(const auto& tSample : tSamples)
    {
        EXPECT_EQ(tCache(tTestVector1, tSample), tSample);
        EXPECT_TRUE(tCache.exists(tTestVector1, tSample));
    }
    EXPECT_TRUE(tCache.exists(tTestVector1));

    const Plato::DistributedVectorROL tTestVector2{MPI_COMM_WORLD, tNumElements, 1.0};
    for(const auto& tSample : tSamples)
    {
        EXPECT_EQ(tCache(tTestVector2, tSample), tSample);
        EXPECT_TRUE(tCache.exists(tTestVector2, tSample));
    }
    EXPECT_TRUE(tCache.exists(tTestVector2));
    EXPECT_FALSE(tCache.exists(tTestVector1));
}

TEST(ROLSampleGeneratorCache, DistributedVectorHash)
{
    constexpr int tVecSize = 10;
    const Plato::DistributedVectorROL<double> tVector{MPI_COMM_WORLD, tVecSize};
    EXPECT_EQ(Plato::detail::rol_vector_hash(tVector), Plato::detail::rol_vector_hash(tVector));

    const Plato::DistributedVectorROL<double> tVector2{MPI_COMM_WORLD, tVecSize, 2};
    EXPECT_NE(Plato::detail::rol_vector_hash(tVector), Plato::detail::rol_vector_hash(tVector2));
}
