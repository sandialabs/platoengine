#include <gtest/gtest.h>

#include <boost/functional/hash.hpp>

#include "plato/utilities/StateCache.hpp"

namespace plato::utilities::unittest
{
TEST(StateCache, StateUpdatedOnlyIfHashChanges)
{
    std::size_t tCallCount{0};

    using DummyCache = plato::utilities::StateCache<std::size_t, std::size_t>;
    auto tDummyCache = DummyCache{[&tCallCount](const std::size_t aArg)
                                  {
                                      tCallCount++;
                                      return aArg;
                                  },
                                  [](std::size_t aArg)
                                  {
                                      boost::hash_combine(aArg, 0);
                                      return aArg;
                                  }};

    ASSERT_EQ(tCallCount, 0);

    EXPECT_EQ(tDummyCache.compute(0), 0);
    EXPECT_EQ(tCallCount, 1);

    EXPECT_EQ(tDummyCache.compute(1), 1);
    EXPECT_EQ(tCallCount, 2);

    EXPECT_EQ(tDummyCache.compute(1), 1);
    EXPECT_EQ(tCallCount, 2);
}
}  // namespace plato::utilities::unittest