#include <gtest/gtest.h>

#include "Plato_SampleResponseCache.hpp"

TEST(SampleResponseCache, SampleHash)
{
    std::hash<Plato::Sample> hasher;
    {
        const Plato::Sample sample1({});
        const Plato::Sample sample2({});
        EXPECT_EQ(hasher(sample1), hasher(sample2));
    }
    {
        const Plato::Sample sample1({1.0, 2.0, 3.0});
        const Plato::Sample sample2({1.0, 2.0, 3.0});
        EXPECT_EQ(hasher(sample1), hasher(sample2));
    }
    {
        const Plato::Sample sample1({3.0, 2.0, 1.0});
        const Plato::Sample sample2({1.0, 2.0, 3.0});
        EXPECT_NE(hasher(sample1), hasher(sample2));
    }
    {
        const Plato::Sample sample1({});
        const Plato::Sample sample2({1.0, 2.0, 3.0});
        EXPECT_NE(hasher(sample1), hasher(sample2));
    }
    {
        const Plato::Sample sample1({1.0});
        const Plato::Sample sample2({1.0, 1.0});
        EXPECT_NE(hasher(sample1), hasher(sample2));
    }
}

TEST(SampleResponseCache, TestOneValue)
{
    Plato::SampleResponseCache<double> tCache;

    const Plato::Sample tSample({0.0, 0.0});
    constexpr double tValue = 4.0;
    tCache.insert(tSample, tValue);
    ASSERT_TRUE(tCache.get(tSample).has_value());
    EXPECT_EQ(tCache.get(tSample).value(), tValue);

    const Plato::Sample tSampleNotInCache({0.0});
    EXPECT_FALSE(tCache.get(tSampleNotInCache).has_value());
}

TEST(SampleResponseCache, TestMultipleValues)
{
    Plato::SampleResponseCache<double> tCache;
    const Plato::Sample tSample1({0.0, 1.0});
    constexpr double tValue1 = 1.0;
    tCache.insert(tSample1, tValue1);
    const Plato::Sample tSample2({1.0, 0.0});
    constexpr double tValue2 = 2.0;
    tCache.insert(tSample2, tValue2);
    const Plato::Sample tSample3({1.0, 1.0});
    constexpr double tValue3 = 3.0;
    tCache.insert(tSample3, tValue3);

    ASSERT_TRUE(tCache.get(tSample1).has_value());
    ASSERT_TRUE(tCache.get(tSample2).has_value());
    ASSERT_TRUE(tCache.get(tSample3).has_value());

    EXPECT_EQ(tCache.get(tSample1).value(), tValue1);
    EXPECT_EQ(tCache.get(tSample2).value(), tValue2);
    EXPECT_EQ(tCache.get(tSample3).value(), tValue3);
}

TEST(SampleResponseCache, TestMultipleValuesWithVectorInsert)
{
    Plato::SampleResponseCache<double> tCache;
    std::vector tSamples = {
        Plato::Sample({0.0, 1.0}),
        Plato::Sample({1.0, 0.0}),
        Plato::Sample({1.0, 1.0}) };
    std::vector tValues = {1.0, 2.0, 3.0};
    tCache.insert(tSamples, tValues);

    ASSERT_TRUE(tCache.get(tSamples[0]).has_value());
    ASSERT_TRUE(tCache.get(tSamples[1]).has_value());
    ASSERT_TRUE(tCache.get(tSamples[2]).has_value());

    EXPECT_EQ(tCache.get(tSamples[0]).value(), tValues[0]);
    EXPECT_EQ(tCache.get(tSamples[1]).value(), tValues[1]);
    EXPECT_EQ(tCache.get(tSamples[2]).value(), tValues[2]);
}

TEST(SampleResponseCache, Clear)
{
    Plato::SampleResponseCache<double> tCache;
    const Plato::Sample tSample1({0.0, 1.0});
    tCache.insert(tSample1, 0.0);
    const Plato::Sample tSample2({1.0, 0.0});
    tCache.insert(tSample2, 0.0);

    EXPECT_TRUE(tCache.get(tSample1).has_value());
    EXPECT_TRUE(tCache.get(tSample2).has_value());

    tCache.clear();
    EXPECT_FALSE(tCache.get(tSample1).has_value());
    EXPECT_FALSE(tCache.get(tSample2).has_value());
}
