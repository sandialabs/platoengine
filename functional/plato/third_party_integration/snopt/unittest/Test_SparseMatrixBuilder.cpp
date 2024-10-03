#include <gtest/gtest.h>

#include "plato/test_utilities/TestContext.hpp"
#include "plato/third_party_integration/snopt/SparseMatrixBuilder.hpp"
#include "plato/third_party_integration/snopt/test_utilities/TestUtilities.hpp"
#include "plato/utilities/Enumerate.hpp"
#include "plato/utilities/IndexRange.hpp"

namespace plato::third_party_integration::snopt::unittest
{
namespace
{
using SparseMatrix = SparseMatrixBuilder<std::size_t, double>;
using Triple = std::tuple<SparseMatrix::Row, SparseMatrix::Column, double>;
}  // namespace

TEST(SparseMatrixBuilder, AppendZero)
{
    auto tSparseMatrixBuilder = SparseMatrix{};

    EXPECT_EQ(tSparseMatrixBuilder.size(), 0U);

    tSparseMatrixBuilder.appendIfNotZero(SparseMatrix::Row{0}, SparseMatrix::Column{0}, 0.0);
    EXPECT_EQ(tSparseMatrixBuilder.size(), 0U);

    tSparseMatrixBuilder.appendIfNotZero(SparseMatrix::Row{100}, SparseMatrix::Column{10}, 0.0);
    EXPECT_EQ(tSparseMatrixBuilder.size(), 0U);
}

TEST(SparseMatrixBuilder, AppendNonZero)
{
    auto tSparseMatrixBuilder = SparseMatrix{};

    const auto tTriple0 = std::make_tuple(SparseMatrix::Row{0}, SparseMatrix::Column{0}, 1.0);
    tSparseMatrixBuilder.appendIfNotZero(std::get<SparseMatrix::Row>(tTriple0),
                                         std::get<SparseMatrix::Column>(tTriple0), std::get<double>(tTriple0));
    EXPECT_EQ(tSparseMatrixBuilder.size(), 1U);
    test_utilities::check_triple<std::size_t, double>(tSparseMatrixBuilder.triple(0), tTriple0,
                                                      TEST_CONTEXT("First triple"));

    const auto tTriple1 = std::make_tuple(SparseMatrix::Row{20}, SparseMatrix::Column{10}, 3.0);
    tSparseMatrixBuilder.appendIfNotZero(std::get<SparseMatrix::Row>(tTriple1),
                                         std::get<SparseMatrix::Column>(tTriple1), std::get<double>(tTriple1));
    EXPECT_EQ(tSparseMatrixBuilder.size(), 2U);
    test_utilities::check_triple<std::size_t, double>(tSparseMatrixBuilder.triple(1), tTriple1,
                                                      TEST_CONTEXT("Second triple"));
}

TEST(SparseMatrixBuilder, AppendRow)
{
    auto tSparseMatrixBuilder = SparseMatrix{};

    const auto tVector1 = std::vector{0.0, 1.0, 0.0, -1.0};
    tSparseMatrixBuilder.appendRow(SparseMatrix::Row{1}, tVector1);

    const auto tVector2 = std::vector{1.0, 0.0, 0.0, 2.0};
    tSparseMatrixBuilder.appendRow(SparseMatrix::Row{2}, tVector2);

    EXPECT_EQ(tSparseMatrixBuilder.size(), 4U);

    const auto tExpectedTriples = std::vector{std::make_tuple(SparseMatrix::Row{1}, SparseMatrix::Column{1}, 1.0),
                                              std::make_tuple(SparseMatrix::Row{1}, SparseMatrix::Column{3}, -1.0),
                                              std::make_tuple(SparseMatrix::Row{2}, SparseMatrix::Column{0}, 1.0),
                                              std::make_tuple(SparseMatrix::Row{2}, SparseMatrix::Column{3}, 2.0)};

    for (const auto [tIndex, tExpectedTriple] : utilities::enumerate(tExpectedTriples))
    {
        test_utilities::check_triple<std::size_t, double>(tExpectedTriple, tSparseMatrixBuilder.triple(tIndex),
                                                          TEST_CONTEXT("Append row triple"));
    }
}

TEST(SparseMatrixBuilder, DataPtrs)
{
    auto tSparseMatrixBuilder = SparseMatrix{};
    constexpr auto tMatrixSize = 4;
    constexpr auto tColumnOffset = 5;
    constexpr auto tEntryMultiplier = -2.0;
    for (const auto tIndex : utilities::IndexRange<unsigned long>{tMatrixSize})
    {
        const auto tEntry = tIndex % 2 == 0 ? 0.0 : static_cast<double>(tIndex);
        tSparseMatrixBuilder.appendIfNotZero(SparseMatrix::Row{tIndex}, SparseMatrix::Column{tIndex + tColumnOffset},
                                             tEntryMultiplier * tEntry);
    }

    ASSERT_EQ(tSparseMatrixBuilder.size(), tMatrixSize / 2);

    for (const auto tIndex : utilities::IndexRange{tSparseMatrixBuilder.size()})
    {
        const auto tOriginalIndex = 2 * tIndex + 1;

        const auto* const tEntryPtr = tSparseMatrixBuilder.entryData().get() + tIndex;
        EXPECT_EQ(*tEntryPtr, tEntryMultiplier * static_cast<double>(tOriginalIndex)) << "Index: " << tIndex;

        const auto* const tRowPtr = tSparseMatrixBuilder.rowData().get() + tIndex;
        EXPECT_EQ(*tRowPtr, tOriginalIndex) << "Index: " << tIndex;

        const auto* const tColumnPtr = tSparseMatrixBuilder.columnData().get() + tIndex;
        EXPECT_EQ(*tColumnPtr, tOriginalIndex + tColumnOffset) << "Index: " << tIndex;
    }
}

}  // namespace plato::third_party_integration::snopt::unittest
