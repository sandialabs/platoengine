#include <gtest/gtest.h>

#include <numeric>

#include "plato/third_party_integration/snopt/SNOPTArray.hpp"
#include "plato/third_party_integration/snopt/test_utilities/TestUtilities.hpp"
#include "plato/utilities/IndexRange.hpp"

namespace plato::third_party_integration::snopt
{
TEST(SNOPTArray, Size)
{
    constexpr auto tSize = 16;
    const auto tArray = std::make_unique<int[]>(tSize);

    const auto tSNOPTArray = SNOPTArray{tArray.get(), tSize};

    EXPECT_EQ(tSNOPTArray.size(), tSize);
}

TEST(SNOPTArray, MutableEntryAccess)
{
    constexpr auto tSize = std::size_t{16};
    const auto tArray = std::make_unique<int[]>(tSize);

    auto tSNOPTArray = SNOPTArray{tArray.get(), tSize};

    constexpr auto tTestValue = 42;
    for (const auto tIndex : utilities::IndexRange{tSize})
    {
        tSNOPTArray[tIndex] = tTestValue + static_cast<int>(tIndex);
    }

    for (const auto tIndex : utilities::IndexRange{tSize})
    {
        EXPECT_EQ(tSNOPTArray[tIndex], tTestValue + static_cast<int>(tIndex));
    }
}

TEST(SNOPTArray, EntryAccess)
{
    constexpr auto tSize = std::size_t{4};
    const auto tArray = test_utilities::array_with_iota<int>(tSize);
    const auto tSNOPTArray = SNOPTArray{tArray.get(), tSize};

    for (const auto tIndex : utilities::IndexRange{tSize})
    {
        EXPECT_EQ(tSNOPTArray[tIndex], tIndex);
    }
}

TEST(SNOPTArray, BeginAndEndConstIterators)
{
    constexpr auto tSize = std::size_t{4};
    const auto tArray = test_utilities::array_with_iota<int>(tSize);
    const auto tSNOPTArray = SNOPTArray{tArray.get(), tSize};

    EXPECT_EQ(tSNOPTArray.begin(), tArray.get());
    EXPECT_EQ(tSNOPTArray.end(), tArray.get() + tSize);

    auto tVector = std::vector<int>(tSize);
    std::copy(tSNOPTArray.begin(), tSNOPTArray.end(), tVector.begin());

    for (const auto tIndex : utilities::IndexRange{tSize})
    {
        EXPECT_EQ(tVector[tIndex], tArray[tIndex]);
    }
}

TEST(SNOPTArray, BeginAndEndIterators)
{
    constexpr auto tSize = std::size_t{4};
    auto tArray = std::make_unique<int[]>(tSize);
    auto tSNOPTArray = SNOPTArray{tArray.get(), tSize};

    EXPECT_EQ(tSNOPTArray.begin(), tArray.get());
    EXPECT_EQ(tSNOPTArray.end(), tArray.get() + tSize);

    std::iota(tSNOPTArray.begin(), tSNOPTArray.end(), 0);
    for (const auto tIndex : utilities::IndexRange{tSize})
    {
        EXPECT_EQ(tSNOPTArray[tIndex], tIndex);
    }
}

}  // namespace plato::third_party_integration::snopt
