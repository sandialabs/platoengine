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
    EXPECT_EQ(enum_index(Numbers::kZero), 0U);
    EXPECT_EQ(enum_index(Numbers::kOne), 1U);
    EXPECT_EQ(enum_index(Numbers::kTwo), 2U);
}

TEST(EnumIndexing, TwoD)
{
    EXPECT_EQ(enum_index(Numbers::kZero, Letters::kA), 0U);
    EXPECT_EQ(enum_index(Numbers::kOne, Letters::kA), 1U);
    EXPECT_EQ(enum_index(Numbers::kTwo, Letters::kA), 2U);

    EXPECT_EQ(enum_index(Numbers::kZero, Letters::kB), 3U);
    EXPECT_EQ(enum_index(Numbers::kOne, Letters::kB), 4U);
    EXPECT_EQ(enum_index(Numbers::kTwo, Letters::kB), 5U);
}

TEST(EnumIndexing, ThreeD)
{
    EXPECT_EQ(enum_index(Numbers::kZero, Letters::kA, Components::kX), 0U);
    EXPECT_EQ(enum_index(Numbers::kOne, Letters::kA, Components::kX), 1U);
    EXPECT_EQ(enum_index(Numbers::kTwo, Letters::kA, Components::kX), 2U);
    EXPECT_EQ(enum_index(Numbers::kZero, Letters::kB, Components::kX), 3U);
    EXPECT_EQ(enum_index(Numbers::kOne, Letters::kB, Components::kX), 4U);
    EXPECT_EQ(enum_index(Numbers::kTwo, Letters::kB, Components::kX), 5U);

    EXPECT_EQ(enum_index(Numbers::kZero, Letters::kA, Components::kY), 6U);
    EXPECT_EQ(enum_index(Numbers::kOne, Letters::kA, Components::kY), 7U);
    EXPECT_EQ(enum_index(Numbers::kTwo, Letters::kA, Components::kY), 8U);
    EXPECT_EQ(enum_index(Numbers::kZero, Letters::kB, Components::kY), 9U);
    EXPECT_EQ(enum_index(Numbers::kOne, Letters::kB, Components::kY), 10U);
    EXPECT_EQ(enum_index(Numbers::kTwo, Letters::kB, Components::kY), 11U);

    EXPECT_EQ(enum_index(Numbers::kZero, Letters::kA, Components::kZ), 12U);
    EXPECT_EQ(enum_index(Numbers::kOne, Letters::kA, Components::kZ), 13U);
    EXPECT_EQ(enum_index(Numbers::kTwo, Letters::kA, Components::kZ), 14U);
    EXPECT_EQ(enum_index(Numbers::kZero, Letters::kB, Components::kZ), 15U);
    EXPECT_EQ(enum_index(Numbers::kOne, Letters::kB, Components::kZ), 16U);
    EXPECT_EQ(enum_index(Numbers::kTwo, Letters::kB, Components::kZ), 17U);
}

TEST(EnumIndexing, EnumsFromINdex1D)
{
    EXPECT_EQ(std::get<0>(enums_from_index<Numbers>(0U)), Numbers::kZero);
    EXPECT_EQ(std::get<0>(enums_from_index<Numbers>(1U)), Numbers::kOne);
    EXPECT_EQ(std::get<0>(enums_from_index<Numbers>(2U)), Numbers::kTwo);
}

TEST(EnumIndexing, EnumsFromINdex2D)
{
    EXPECT_EQ(std::get<0>(enums_from_index<Numbers, Letters>(0U)), Numbers::kZero);
    EXPECT_EQ(std::get<0>(enums_from_index<Numbers, Letters>(1U)), Numbers::kOne);
    EXPECT_EQ(std::get<0>(enums_from_index<Numbers, Letters>(2U)), Numbers::kTwo);
    EXPECT_EQ(std::get<1>(enums_from_index<Numbers, Letters>(0U)), Letters::kA);
    EXPECT_EQ(std::get<1>(enums_from_index<Numbers, Letters>(1U)), Letters::kA);
    EXPECT_EQ(std::get<1>(enums_from_index<Numbers, Letters>(2U)), Letters::kA);

    EXPECT_EQ(std::get<0>(enums_from_index<Numbers, Letters>(3U)), Numbers::kZero);
    EXPECT_EQ(std::get<0>(enums_from_index<Numbers, Letters>(4U)), Numbers::kOne);
    EXPECT_EQ(std::get<0>(enums_from_index<Numbers, Letters>(5U)), Numbers::kTwo);
    EXPECT_EQ(std::get<1>(enums_from_index<Numbers, Letters>(3U)), Letters::kB);
    EXPECT_EQ(std::get<1>(enums_from_index<Numbers, Letters>(4U)), Letters::kB);
    EXPECT_EQ(std::get<1>(enums_from_index<Numbers, Letters>(5U)), Letters::kB);
}

TEST(EnumIndexing, EnumsFromINdex3D)
{
    EXPECT_EQ((enums_from_index<Numbers, Letters, Components>(0U)),
              std::make_tuple(Numbers::kZero, Letters::kA, Components::kX));
    EXPECT_EQ((enums_from_index<Numbers, Letters, Components>(1U)),
              std::make_tuple(Numbers::kOne, Letters::kA, Components::kX));
    EXPECT_EQ((enums_from_index<Numbers, Letters, Components>(2U)),
              std::make_tuple(Numbers::kTwo, Letters::kA, Components::kX));
    EXPECT_EQ((enums_from_index<Numbers, Letters, Components>(3U)),
              std::make_tuple(Numbers::kZero, Letters::kB, Components::kX));
    EXPECT_EQ((enums_from_index<Numbers, Letters, Components>(4U)),
              std::make_tuple(Numbers::kOne, Letters::kB, Components::kX));
    EXPECT_EQ((enums_from_index<Numbers, Letters, Components>(5U)),
              std::make_tuple(Numbers::kTwo, Letters::kB, Components::kX));

    EXPECT_EQ((enums_from_index<Numbers, Letters, Components>(6U)),
              std::make_tuple(Numbers::kZero, Letters::kA, Components::kY));
    EXPECT_EQ((enums_from_index<Numbers, Letters, Components>(7U)),
              std::make_tuple(Numbers::kOne, Letters::kA, Components::kY));
    EXPECT_EQ((enums_from_index<Numbers, Letters, Components>(8U)),
              std::make_tuple(Numbers::kTwo, Letters::kA, Components::kY));
    EXPECT_EQ((enums_from_index<Numbers, Letters, Components>(9U)),
              std::make_tuple(Numbers::kZero, Letters::kB, Components::kY));
    EXPECT_EQ((enums_from_index<Numbers, Letters, Components>(10U)),
              std::make_tuple(Numbers::kOne, Letters::kB, Components::kY));
    EXPECT_EQ((enums_from_index<Numbers, Letters, Components>(11U)),
              std::make_tuple(Numbers::kTwo, Letters::kB, Components::kY));

    EXPECT_EQ((enums_from_index<Numbers, Letters, Components>(12U)),
              std::make_tuple(Numbers::kZero, Letters::kA, Components::kZ));
    EXPECT_EQ((enums_from_index<Numbers, Letters, Components>(13U)),
              std::make_tuple(Numbers::kOne, Letters::kA, Components::kZ));
    EXPECT_EQ((enums_from_index<Numbers, Letters, Components>(14U)),
              std::make_tuple(Numbers::kTwo, Letters::kA, Components::kZ));
    EXPECT_EQ((enums_from_index<Numbers, Letters, Components>(15U)),
              std::make_tuple(Numbers::kZero, Letters::kB, Components::kZ));
    EXPECT_EQ((enums_from_index<Numbers, Letters, Components>(160U)),
              std::make_tuple(Numbers::kOne, Letters::kB, Components::kZ));
    EXPECT_EQ((enums_from_index<Numbers, Letters, Components>(17U)),
              std::make_tuple(Numbers::kTwo, Letters::kB, Components::kZ));
}

TEST(EnumIndexing, NumberOfEnumerates)
{
    EXPECT_EQ(number_of_enumerates<Numbers>(), 3U);
    EXPECT_EQ((number_of_enumerates<Numbers, Letters>()), 6U);
    EXPECT_EQ((number_of_enumerates<Letters, Numbers>()), 6U);
    EXPECT_EQ((number_of_enumerates<Numbers, Letters, Components>()), 18U);
}

}  // namespace plato::utilities::unittest
