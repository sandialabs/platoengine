#include <gtest/gtest.h>

#include "plato/test_utilities/TestContext.hpp"
#include "plato/utilities/EnumIndexing.hpp"
#include "plato/utilities/MultidimensionalRange.hpp"

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

template <typename TestFunction>
void check_3d_enum_indexing(const TestFunction& aTestFunction, const test_utilities::TestContext& aTestContext)
{
    auto tLinearIndex = 0U;
    for (const auto& [tComponentValue, tLetterValue, tNumberValue] : MultidimensionalRange{
             number_of_enumerates<Components>(), number_of_enumerates<Letters>(), number_of_enumerates<Numbers>()})
    {
        const auto tNumber = static_cast<Numbers>(tNumberValue);
        const auto tLetter = static_cast<Letters>(tLetterValue);
        const auto tComponent = static_cast<Components>(tComponentValue);
        aTestFunction(tNumber, tLetter, tComponent, tLinearIndex, aTestContext);
        ++tLinearIndex;
    }
    EXPECT_EQ(tLinearIndex, (number_of_enumerates<Numbers, Letters, Components>())) << aTestContext;
}

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
    const auto tTestFunction = [](const Numbers aNumber, const Letters aLetter, const Components aComponent,
                                  const std::size_t aLinearIndex, const test_utilities::TestContext& aTestContext)
    { EXPECT_EQ(enum_index(aNumber, aLetter, aComponent), aLinearIndex) << aTestContext; };

    check_3d_enum_indexing(tTestFunction, TEST_CONTEXT("Enums to index"));
}

TEST(EnumIndexing, EnumsFromINdex1D)
{
    EXPECT_EQ(std::get<0>(enums_from_index<Numbers>(0U)), Numbers::kZero);
    EXPECT_EQ(std::get<0>(enums_from_index<Numbers>(1U)), Numbers::kOne);
    EXPECT_EQ(std::get<0>(enums_from_index<Numbers>(2U)), Numbers::kTwo);
}

TEST(EnumIndexing, EnumsFromINdex2D)
{
    EXPECT_EQ((enums_from_index<Numbers, Letters>(0U)), std::make_tuple(Numbers::kZero, Letters::kA));
    EXPECT_EQ((enums_from_index<Numbers, Letters>(1U)), std::make_tuple(Numbers::kOne, Letters::kA));
    EXPECT_EQ((enums_from_index<Numbers, Letters>(2U)), std::make_tuple(Numbers::kTwo, Letters::kA));

    EXPECT_EQ((enums_from_index<Numbers, Letters>(3U)), std::make_tuple(Numbers::kZero, Letters::kB));
    EXPECT_EQ((enums_from_index<Numbers, Letters>(4U)), std::make_tuple(Numbers::kOne, Letters::kB));
    EXPECT_EQ((enums_from_index<Numbers, Letters>(5U)), std::make_tuple(Numbers::kTwo, Letters::kB));
}

TEST(EnumIndexing, EnumsFromIndex3D)
{
    const auto tTestFunction = [](const Numbers aNumber, const Letters aLetter, const Components aComponent,
                                  const std::size_t aLinearIndex, const test_utilities::TestContext& aTestContext)
    {
        EXPECT_EQ((enums_from_index<Numbers, Letters, Components>(aLinearIndex)),
                  std::make_tuple(aNumber, aLetter, aComponent))
            << aTestContext;
    };

    check_3d_enum_indexing(tTestFunction, TEST_CONTEXT("Index to enums"));
}

TEST(EnumIndexing, NumberOfEnumerates)
{
    EXPECT_EQ(number_of_enumerates<Numbers>(), 3U);
    EXPECT_EQ((number_of_enumerates<Numbers, Letters>()), 6U);
    EXPECT_EQ((number_of_enumerates<Letters, Numbers>()), 6U);
    EXPECT_EQ((number_of_enumerates<Numbers, Letters, Components>()), 18U);
}

}  // namespace plato::utilities::unittest
