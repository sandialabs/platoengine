#include <gtest/gtest.h>

#include "plato/utilities/EnumIndexing.hpp"

namespace plato::utilities::unittest
{
namespace
{
enum struct Numbers
{
    kZero,
    kOne,
    kTwo,
    kNumberOfEnumerates,
};

enum struct Letters
{
    kA,
    kB,
    kNumberOfEnumerates,
};

enum struct Components
{
    kX,
    kY,
    kZ,
    kNumberOfEnumerates,
};

}  // namespace

TEST(EnumIndexing, DimensionStride)
{
    EXPECT_EQ((detail::dimension_stride<0U, Numbers>()), 1U);

    EXPECT_EQ((detail::dimension_stride<0U, Numbers, Letters>()), 1U);
    EXPECT_EQ((detail::dimension_stride<1U, Numbers, Letters>()), 3U);

    EXPECT_EQ((detail::dimension_stride<0U, Numbers, Letters, Components>()), 1U);
    EXPECT_EQ((detail::dimension_stride<1U, Numbers, Letters, Components>()), 3U);
    EXPECT_EQ((detail::dimension_stride<2U, Numbers, Letters, Components>()), 6U);
}

TEST(EnumIndexing, OneD)
{
    EXPECT_EQ(enum_index<Numbers::kZero>(), 0U);
    EXPECT_EQ(enum_index<Numbers::kOne>(), 1U);
    EXPECT_EQ(enum_index<Numbers::kTwo>(), 2U);
}

TEST(EnumIndexing, TwoD)
{
    EXPECT_EQ((enum_index<Numbers::kZero, Letters::kA>()), 0U);
    EXPECT_EQ((enum_index<Numbers::kOne, Letters::kA>()), 1U);
    EXPECT_EQ((enum_index<Numbers::kTwo, Letters::kA>()), 2U);

    EXPECT_EQ((enum_index<Numbers::kZero, Letters::kB>()), 3U);
    EXPECT_EQ((enum_index<Numbers::kOne, Letters::kB>()), 4U);
    EXPECT_EQ((enum_index<Numbers::kTwo, Letters::kB>()), 5U);
}

TEST(EnumIndexing, ThreeD)
{
    EXPECT_EQ((enum_index<Numbers::kZero, Letters::kA, Components::kX>()), 0U);
    EXPECT_EQ((enum_index<Numbers::kOne, Letters::kA, Components::kX>()), 1U);
    EXPECT_EQ((enum_index<Numbers::kTwo, Letters::kA, Components::kX>()), 2U);
    EXPECT_EQ((enum_index<Numbers::kZero, Letters::kB, Components::kX>()), 3U);
    EXPECT_EQ((enum_index<Numbers::kOne, Letters::kB, Components::kX>()), 4U);
    EXPECT_EQ((enum_index<Numbers::kTwo, Letters::kB, Components::kX>()), 5U);

    EXPECT_EQ((enum_index<Numbers::kZero, Letters::kA, Components::kY>()), 6U);
    EXPECT_EQ((enum_index<Numbers::kOne, Letters::kA, Components::kY>()), 7U);
    EXPECT_EQ((enum_index<Numbers::kTwo, Letters::kA, Components::kY>()), 8U);
    EXPECT_EQ((enum_index<Numbers::kZero, Letters::kB, Components::kY>()), 9U);
    EXPECT_EQ((enum_index<Numbers::kOne, Letters::kB, Components::kY>()), 10U);
    EXPECT_EQ((enum_index<Numbers::kTwo, Letters::kB, Components::kY>()), 11U);

    EXPECT_EQ((enum_index<Numbers::kZero, Letters::kA, Components::kZ>()), 12U);
    EXPECT_EQ((enum_index<Numbers::kOne, Letters::kA, Components::kZ>()), 13U);
    EXPECT_EQ((enum_index<Numbers::kTwo, Letters::kA, Components::kZ>()), 14U);
    EXPECT_EQ((enum_index<Numbers::kZero, Letters::kB, Components::kZ>()), 15U);
    EXPECT_EQ((enum_index<Numbers::kOne, Letters::kB, Components::kZ>()), 16U);
    EXPECT_EQ((enum_index<Numbers::kTwo, Letters::kB, Components::kZ>()), 17U);
}

}  // namespace plato::utilities::unittest
