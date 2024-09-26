#include <gtest/gtest.h>

#include "plato/third_party_integration/snopt/DataSingleton.hpp"

namespace plato::third_party_integration::snopt::unittest
{
namespace
{
struct TestTag
{
};

struct AnotherTag
{
};
}  // namespace

TEST(DataSingleton, WrapsIntNoTag)
{
    auto& tSingleton = DataSingleton<int>::instance();
    EXPECT_FALSE(tSingleton.data().has_value());

    constexpr auto tTestValue = 42;
    tSingleton.data() = tTestValue;
    ASSERT_TRUE(tSingleton.data().has_value());
    EXPECT_EQ(tSingleton.data().value(), tTestValue);
}

TEST(DataSingleton, TwoTags)
{
    auto& tSingleton = DataSingleton<int, TestTag>::instance();
    EXPECT_FALSE(tSingleton.data().has_value());

    auto& tAnotherSingleton = DataSingleton<int, AnotherTag>::instance();
    EXPECT_FALSE(tAnotherSingleton.data().has_value());

    constexpr auto tTestValue = 42;
    tSingleton.data() = tTestValue;
    constexpr auto tAnotherTestValue = 13;
    tAnotherSingleton.data() = tAnotherTestValue;

    EXPECT_EQ((DataSingleton<int, TestTag>::instance().data().value()), tTestValue);
    EXPECT_EQ((DataSingleton<int, AnotherTag>::instance().data().value()), tAnotherTestValue);
}

}  // namespace plato::third_party_integration::snopt::unittest
