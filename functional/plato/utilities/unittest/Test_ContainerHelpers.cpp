#include <gtest/gtest.h>

#include <unordered_set>
#include <vector>

#include "plato/utilities/ContainerHelpers.hpp"

namespace plato::utilities::unittest
{
TEST(ContainerHelpers, ReservedContainer)
{
    constexpr auto tCapacity = std::size_t{42U};
    // vector
    {
        const auto tReservedVector = reserved_container<std::vector<double>>(tCapacity);
        EXPECT_EQ(tReservedVector.capacity(), tCapacity);
    }
    // unordered_set
    {
        const auto tReservedSet = reserved_container<std::unordered_set<int>>(tCapacity);
        EXPECT_EQ(tReservedSet.bucket_count() - 1, tCapacity);
    }
}

TEST(ContainerHelpers, FindKeyWithValue)
{
    const auto tMap = std::map<int, std::string>{{0, "0"}, {10, "10"}, {5, "5"}};

    ASSERT_FALSE(key_with_value(tMap, "42"));

    ASSERT_TRUE(key_with_value(tMap, "0"));
    EXPECT_EQ(key_with_value(tMap, "0").value(), 0);

    ASSERT_TRUE(key_with_value(tMap, "5"));
    EXPECT_EQ(key_with_value(tMap, "5").value(), 5);

    ASSERT_TRUE(key_with_value(tMap, "10"));
    EXPECT_EQ(key_with_value(tMap, "10").value(), 10);
}

}  // namespace plato::utilities::unittest
