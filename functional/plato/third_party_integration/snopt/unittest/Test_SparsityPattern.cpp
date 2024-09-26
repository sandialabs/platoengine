#include <gtest/gtest.h>

#include "plato/test_utilities/TestContext.hpp"
#include "plato/third_party_integration/snopt/SparsityPattern.hpp"
#include "plato/utilities/Enumerate.hpp"
#include "plato/utilities/IndexRange.hpp"

namespace plato::third_party_integration::snopt::unittest
{
namespace
{
using TestSparsityPattern = SparsityPattern<std::size_t>;
using RowAndColumn = std::tuple<TestSparsityPattern::Row, TestSparsityPattern::Column>;

void check_row_and_column(const RowAndColumn& aLeft,
                          const RowAndColumn& aRight,
                          const plato::test_utilities::TestContext& aTestContext)
{
    EXPECT_EQ(std::get<TestSparsityPattern::Row>(aLeft).mValue, std::get<TestSparsityPattern::Row>(aRight).mValue)
        << aTestContext;
    EXPECT_EQ(std::get<TestSparsityPattern::Column>(aLeft).mValue, std::get<TestSparsityPattern::Column>(aRight).mValue)
        << aTestContext;
}
}  // namespace

TEST(SparsityPattern, Append)
{
    auto tSparsityPattern = TestSparsityPattern{};

    const auto tRowAndColumn0 = std::make_pair(TestSparsityPattern::Row{0}, TestSparsityPattern::Column{0});
    tSparsityPattern.append(std::get<TestSparsityPattern::Row>(tRowAndColumn0),
                            std::get<TestSparsityPattern::Column>(tRowAndColumn0));
    EXPECT_EQ(tSparsityPattern.size(), 1U);
    check_row_and_column(tSparsityPattern.rowAndColumn(0), tRowAndColumn0, TEST_CONTEXT("First row and column"));

    const auto tRowAndColumn1 = std::make_pair(TestSparsityPattern::Row{20}, TestSparsityPattern::Column{10});
    tSparsityPattern.append(std::get<TestSparsityPattern::Row>(tRowAndColumn1),
                            std::get<TestSparsityPattern::Column>(tRowAndColumn1));
    EXPECT_EQ(tSparsityPattern.size(), 2U);
    check_row_and_column(tSparsityPattern.rowAndColumn(1), tRowAndColumn1, TEST_CONTEXT("Second row and column"));
}

TEST(SparsityPattern, offsetRows)
{
    auto tSparsityPattern = TestSparsityPattern{};

    const auto tRowsAndColumns =
        std::vector{std::make_pair(TestSparsityPattern::Row{0}, TestSparsityPattern::Column{0}),
                    std::make_pair(TestSparsityPattern::Row{12}, TestSparsityPattern::Column{3})};

    for (const auto& [tRow, tColumn] : tRowsAndColumns)
    {
        tSparsityPattern.append(tRow, tColumn);
    }

    constexpr auto tOffset = TestSparsityPattern::Row{42};
    tSparsityPattern.offsetRows(tOffset);

    for (const auto [tIndex, tRowAndColumn] : utilities::enumerate(tRowsAndColumns))
    {
        const auto tExpectedRowAndColumn =
            std::make_pair(TestSparsityPattern::Row{tRowAndColumn.first.mValue + tOffset.mValue}, tRowAndColumn.second);
        check_row_and_column(tSparsityPattern.rowAndColumn(tIndex), tExpectedRowAndColumn,
                             TEST_CONTEXT("Row with offset"));
    }
}

TEST(SparsityPattern, DataPtrs)
{
    auto tSparsityPattern = TestSparsityPattern{};
    constexpr auto tMatrixSize = 4;
    constexpr auto tColumnOffset = 5;
    for (const auto tIndex : utilities::IndexRange{tMatrixSize})
    {
        tSparsityPattern.append(TestSparsityPattern::Row{tIndex}, TestSparsityPattern::Column{tIndex + tColumnOffset});
    }

    ASSERT_EQ(tSparsityPattern.size(), tMatrixSize);

    for (const auto tIndex : utilities::IndexRange{tSparsityPattern.size()})
    {
        const auto* const tRowPtr = tSparsityPattern.rowData().get() + tIndex;
        EXPECT_EQ(*tRowPtr, tIndex);

        const auto* const tColumnPtr = tSparsityPattern.columnData().get() + tIndex;
        EXPECT_EQ(*tColumnPtr, tIndex + tColumnOffset);
    }
}

TEST(SparsityPattern, DenseMatrix)
{
    constexpr auto tNumberOfRows = std::size_t{2};
    constexpr auto tNumberOfColumns = std::size_t{3};
    constexpr auto tStartingIndex = std::size_t{2};
    const auto tDenseMatrixPattern = dense_matrix_sparsity_pattern(
        TestSparsityPattern::Row{tNumberOfRows}, TestSparsityPattern::Column{tNumberOfColumns}, tStartingIndex);

    EXPECT_EQ(tDenseMatrixPattern.size(), tNumberOfRows * tNumberOfColumns);

    auto tRow = std::size_t{0};
    auto tColumn = std::size_t{0};
    for (const auto tIndex : utilities::IndexRange{tDenseMatrixPattern.size()})
    {
        const auto tRowAndColumn = tDenseMatrixPattern.rowAndColumn(tIndex);
        EXPECT_EQ(tRowAndColumn.first.mValue, tRow + tStartingIndex);
        EXPECT_EQ(tRowAndColumn.second.mValue, tColumn + tStartingIndex);
        ++tColumn;
        if (tColumn == tNumberOfColumns)
        {
            tColumn = std::size_t{0};
            ++tRow;
        }
    }
}

}  // namespace plato::third_party_integration::snopt::unittest
