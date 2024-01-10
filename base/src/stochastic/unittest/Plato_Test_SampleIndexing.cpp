#include <gtest/gtest.h>

#include "Plato_SampleIndexing.hpp"

namespace
{
void test_full_loop(const Plato::NumPerformers aNumPerformers, const Plato::NumSamples aNumSamples)
{
    const Plato::SampleIndexing tIndexer{aNumPerformers, aNumSamples};
    std::size_t tSampleIndexFromLoop = 0;
    for(const Plato::SampleBatch tBatch : tIndexer)
    {
        std::size_t tPerformerIndexFromLoop = 0;
        for(const auto [tPerformerIndexFromIterator, tSampleIndexFromIterator] : tBatch)
        {
            EXPECT_EQ(tPerformerIndexFromIterator, tPerformerIndexFromLoop);
            EXPECT_EQ(tSampleIndexFromIterator, tSampleIndexFromLoop);
            tSampleIndexFromLoop = tSampleIndexFromLoop == aNumSamples.mValue - 1 ? 0 : tSampleIndexFromLoop + 1;
            ++tPerformerIndexFromLoop;
        }
        EXPECT_EQ(tPerformerIndexFromLoop, aNumPerformers.mValue);
    }
    const std::size_t tNumExtraEvals = 
        aNumSamples.mValue % aNumPerformers.mValue == 0 ? 0 : aNumSamples.mValue % aNumPerformers.mValue - 1;
    EXPECT_EQ(tSampleIndexFromLoop, tNumExtraEvals);
}
}

TEST(SampleIndexing, PerformerIndexIterator)
{
    constexpr auto tNumPerformers = Plato::NumPerformers{2};
    constexpr auto tNumSamples = Plato::NumSamples{10};
    constexpr std::size_t tSampleIndex = 2;

    Plato::PerformerIndexIterator tIterator{tNumPerformers, tNumSamples, tSampleIndex};
    EXPECT_EQ((*tIterator).mPerformerIndex, 0);
    EXPECT_EQ((*tIterator).mSampleIndex, tSampleIndex);

    const auto tIteratorOriginal = tIterator;
    ++tIterator;
    EXPECT_EQ((*tIterator).mPerformerIndex, 1);
    EXPECT_EQ((*tIterator).mSampleIndex, tSampleIndex + 1);

    EXPECT_NE(tIterator, tIteratorOriginal);
}

TEST(SampleIndexing, SampleBatch)
{
    constexpr auto tNumPerformers = Plato::NumPerformers{2};
    constexpr auto tNumSamples = Plato::NumSamples{10};
    constexpr std::size_t tBatchIndex = 2;

    Plato::SampleBatch tBatch{tNumPerformers, tNumSamples, tBatchIndex};

    EXPECT_NE(tBatch.begin(), tBatch.end());
}

TEST(SampleIndexing, SampleBatchIterator)
{
    constexpr auto tNumPerformers = Plato::NumPerformers{7};
    constexpr auto tNumSamples = Plato::NumSamples{49};
    constexpr std::size_t tBatchIndex = 2;

    Plato::SampleBatchIterator tIterator1{tNumPerformers, tNumSamples, tBatchIndex};
    Plato::SampleBatchIterator tIterator2 = tIterator1;
    ++tIterator1;

    EXPECT_NE(tIterator1, tIterator2);
}

TEST(SampleIndexing, SinglePerformer)
{
    constexpr auto tNumPerformers = Plato::NumPerformers{1};
    constexpr auto tNumSamples = Plato::NumSamples{5};
    test_full_loop(tNumPerformers, tNumSamples);
}

TEST(SampleIndexing, Divisible)
{
    constexpr auto tNumPerformers = Plato::NumPerformers{3};
    constexpr auto tNumSamples = Plato::NumSamples{9};
    test_full_loop(tNumPerformers, tNumSamples);
}

TEST(SampleIndexing, Indivisible)
{
    constexpr auto tNumPerformers = Plato::NumPerformers{3};
    constexpr auto tNumSamples = Plato::NumSamples{11};
    test_full_loop(tNumPerformers, tNumSamples);
}
