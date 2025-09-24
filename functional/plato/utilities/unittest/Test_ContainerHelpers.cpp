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
}  // namespace plato::utilities::unittest
