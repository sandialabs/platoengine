#ifndef PLATO_THIRD_PARTY_INTEGRATION_SNOPT_SPARSITYPATTERN
#define PLATO_THIRD_PARTY_INTEGRATION_SNOPT_SPARSITYPATTERN

#include <algorithm>
#include <numeric>
#include <type_traits>
#include <vector>

#include "plato/third_party_integration/snopt/NonNullPtr.hpp"
#include "plato/third_party_integration/snopt/VectorWithValidDataPtr.hpp"
#include "plato/utilities/IndexRange.hpp"
#include "plato/utilities/NamedType.hpp"

namespace plato::third_party_integration::snopt
{
/// @brief Manages a sparse matrix pattern using row and column entries.
/// @tparam IndexType The type used for the row and colum indices, must satisfy `std::is_integral`
template <typename IndexType>
class SparsityPattern
{
    static_assert(std::is_integral_v<IndexType>, "The IndexType template parameter must be an integral type.");

   public:
    using Row = utilities::NamedType<IndexType, struct RowTag>;
    using Column = utilities::NamedType<IndexType, struct ColumnTag>;
    using RowVector = utilities::NamedType<VectorWithValidDataPtr<IndexType>, struct RowVectorTag>;
    using ColumnVector = utilities::NamedType<VectorWithValidDataPtr<IndexType>, struct ColumnVectorTag>;

    SparsityPattern() = default;

    /// @brief Construction from existing row and column index vectors.
    /// @pre The size of @a aRowVector must be equal to the size of @a aColumnVector.
    SparsityPattern(RowVector aRowVector, ColumnVector aColumnVector);

    /// @brief Appends a row and column pair to the sparsity pattern.
    void append(Row aRow, Column aColumn);

    /// @brief Adds @a aRowOffset to all row indices.
    void offsetRows(Row aRowOffset);

    /// @brief The total number of row/column pairs.
    [[nodiscard]] auto size() const -> std::size_t;
    /// @brief The row/column pair at index @a aIndex
    [[nodiscard]] auto rowAndColumn(std::size_t aIndex) const -> std::pair<Row, Column>;

    /// @brief Returns a pointer to the underlying data array holding the rows.
    [[nodiscard]] auto rowData() -> NonNullPtr<IndexType>;
    /// @brief Returns a pointer to the underlying data array holding the columns.
    [[nodiscard]] auto columnData() -> NonNullPtr<IndexType>;

   private:
    VectorWithValidDataPtr<IndexType> mRows;
    VectorWithValidDataPtr<IndexType> mColumns;
};

/// @brief Creates a dense matrix sparsity pattern with number of rows @a aNumberOfRows and number of columns @a
/// aNumberOfColumns.
/// @param aStartingIndex The index representing the first row/column, should be 0 or 1 depending on the convention
/// used.
template <typename IndexType>
auto dense_matrix_sparsity_pattern(typename SparsityPattern<IndexType>::Row aNumberOfRows,
                                   typename SparsityPattern<IndexType>::Column aNumberOfColumns,
                                   IndexType aStartingIndex) -> SparsityPattern<IndexType>;

template <typename IndexType>
SparsityPattern<IndexType>::SparsityPattern(RowVector aRowVector, ColumnVector aColumnVector)
    : mRows{std::move(aRowVector.mValue)}, mColumns{std::move(aColumnVector.mValue)}
{
}

template <typename IndexType>
void SparsityPattern<IndexType>::append(const Row aRow, const Column aColumn)
{
    mRows.pushBack(aRow.mValue);
    mColumns.pushBack(aColumn.mValue);
}

template <typename IndexType>
void SparsityPattern<IndexType>::offsetRows(Row aRowOffset)
{
    std::transform(mRows.data(), mRows.data() + mRows.size(), mRows.data(),
                   [aRowOffset](const auto& aRow) { return aRow + aRowOffset.mValue; });
}

template <typename IndexType>
auto SparsityPattern<IndexType>::size() const -> std::size_t
{
    return mRows.size();
}

template <typename IndexType>
auto SparsityPattern<IndexType>::rowAndColumn(const std::size_t aIndex) const -> std::pair<Row, Column>
{
    return {Row{mRows[aIndex]}, Column{mColumns[aIndex]}};
}

template <typename IndexType>
auto SparsityPattern<IndexType>::rowData() -> NonNullPtr<IndexType>
{
    return NonNullPtr{mRows.data()};
}

template <typename IndexType>
auto SparsityPattern<IndexType>::columnData() -> NonNullPtr<IndexType>
{
    return NonNullPtr{mColumns.data()};
}

template <typename IndexType>
auto dense_matrix_sparsity_pattern(typename SparsityPattern<IndexType>::Row aNumberOfRows,
                                   typename SparsityPattern<IndexType>::Column aNumberOfColumns,
                                   IndexType aStartingIndex) -> SparsityPattern<IndexType>
{
    using RowVector = typename SparsityPattern<IndexType>::RowVector;
    using ColumnVector = typename SparsityPattern<IndexType>::ColumnVector;

    const auto tTotalEntries = aNumberOfRows.mValue * aNumberOfColumns.mValue;
    auto tRows = VectorWithValidDataPtr<IndexType>(tTotalEntries);
    auto tColumns = VectorWithValidDataPtr<IndexType>(tTotalEntries);

    auto tRowIterator = tRows.data();
    auto tColumnIterator = tColumns.data();
    for (const auto tRow : utilities::IndexRange{aNumberOfRows.mValue})
    {
        std::fill(tRowIterator, std::next(tRowIterator, aNumberOfColumns.mValue), tRow + aStartingIndex);
        std::iota(tColumnIterator, std::next(tColumnIterator, aNumberOfColumns.mValue), aStartingIndex);
        std::advance(tRowIterator, aNumberOfColumns.mValue);
        std::advance(tColumnIterator, aNumberOfColumns.mValue);
    }
    return SparsityPattern<IndexType>{RowVector{std::move(tRows)}, ColumnVector{std::move(tColumns)}};
}

}  // namespace plato::third_party_integration::snopt

#endif
