#include <gtest/gtest.h>

#include <set>
#include <vector>

#include "plato/utilities/Enumerate.hpp"

namespace plato::utilities::unittest
{

TEST(Enumerate, OneVectorMatchingEntries)
{
    const auto tEntries = std::vector<unsigned int>{0, 1, 2, 3, 4};
    for (const auto& [tIndex, tEntry] : utilities::enumerate(entries))
    {
        EXPECT_EQ(tIndex, tEntry);
    }
}

TEST(Enumerate, SetAndVector)
{
    const auto tUnsignedIntSet = std::set<unsigned int>{0, 1, 2, 3, 4};
    const auto tDoubleVector = std::vector<double>{0.0, -1.0, -2.0, -3.0, -4.0};
    for (const auto& [tIndex, tUnsignedIntFromSet, tDoubleFromVector] :
         utilities::enumerate(tUnsignedIntSet, tDoubleVector))
    {
        EXPECT_EQ(tIndex, tUnsignedIntFromSet);
        EXPECT_EQ(-static_cast<double>(tIndex), tDoubleFromVector);
    }
}

}  // namespace plato::utilities::unittest
