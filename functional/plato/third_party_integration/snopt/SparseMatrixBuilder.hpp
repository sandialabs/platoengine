#ifndef PLATO_THIRD_PARTY_INTEGRATION_SNOPT_SPARSEMATRIXBUILDER
#define PLATO_THIRD_PARTY_INTEGRATION_SNOPT_SPARSEMATRIXBUILDER

#include <boost/numeric/conversion/cast.hpp>
#include <tuple>
#include <type_traits>
#include <vector>

#include "plato/third_party_integration/snopt/NonNullPtr.hpp"
#include "plato/third_party_integration/snopt/SparsityPattern.hpp"
#include "plato/third_party_integration/snopt/VectorWithValidDataPtr.hpp"
#include "plato/utilities/Enumerate.hpp"
#include "plato/utilities/NamedType.hpp"

namespace plato::third_party_integration::snopt
{

/// @brief A helper class for forming a sparse matrix from row, column, entry triples.
/// @tparam IndexType The type used for the row and colum indices, must satisfy `std::is_integral`
/// @tparam EntryType The type used for the matrix entries, must satisfy `std::is_arithmetic`
template <typename IndexType, typename EntryType>
class SparseMatrixBuilder
{
    static_assert(std::is_arithmetic_v<EntryType>, "The EntryType template parameter must be an arithmetic type.");

   public:
    using Row = typename SparsityPattern<IndexType>::Row;
    using Column = typename SparsityPattern<IndexType>::Column;

    /// @brief Appends a new entry to the sparse matrix if and only if @a aEntry is not 0.
    void appendIfNotZero(Row aRow, Column aColumn, EntryType aEntry);

    /// @brief Appends new entries from the vector @a aRow for all non-zero entries and uses the index as the column
    /// index.
    void appendRow(Row aRowIndex, const std::vector<EntryType>& aRow);

    /// @brief Const access to the sparsity pattern.
    auto sparsityPattern() const -> const SparsityPattern<IndexType>&;
    /// @brief Non-const access to the sparsity pattern.
    auto sparsityPattern() -> SparsityPattern<IndexType>&;

    /// @brief The total number of entries.
    [[nodiscard]] auto size() const -> std::size_t;
    /// @brief Returns the row/column/entry triple at index @a aIndex.
    [[nodiscard]] auto triple(std::size_t aIndex) const -> std::tuple<Row, Column, EntryType>;

    /// @brief Returns a pointer to the underlying data array holding the row indices.
    [[nodiscard]] auto rowData() -> NonNullPtr<IndexType>;
    /// @brief Returns a pointer to the underlying data array holding the column indices.
    [[nodiscard]] auto columnData() -> NonNullPtr<IndexType>;
    /// @brief Returns a pointer to the underlying data array holding the entries.
    [[nodiscard]] auto entryData() -> NonNullPtr<EntryType>;

   private:
    SparsityPattern<IndexType> mPattern;
    VectorWithValidDataPtr<EntryType> mEntries;
};

template <typename IndexType, typename EntryType>
void SparseMatrixBuilder<IndexType, EntryType>::appendIfNotZero(const Row aRow,
                                                                const Column aColumn,
                                                                const EntryType aEntry)
{
    if (aEntry != EntryType{})
    {
        mPattern.append(aRow, aColumn);
        mEntries.pushBack(aEntry);
    }
}

template <typename IndexType, typename EntryType>
void SparseMatrixBuilder<IndexType, EntryType>::appendRow(const Row aRowIndex, const std::vector<EntryType>& aRow)
{
    for (const auto [tIndex, aRowEntry] : utilities::enumerate(aRow))
    {
        appendIfNotZero(aRowIndex, Column{boost::numeric_cast<IndexType>(tIndex)}, aRowEntry);
    }
}

template <typename IndexType, typename EntryType>
auto SparseMatrixBuilder<IndexType, EntryType>::sparsityPattern() const -> const SparsityPattern<IndexType>&
{
    return mPattern;
}

template <typename IndexType, typename EntryType>
auto SparseMatrixBuilder<IndexType, EntryType>::sparsityPattern() -> SparsityPattern<IndexType>&
{
    return mPattern;
}

template <typename IndexType, typename EntryType>
auto SparseMatrixBuilder<IndexType, EntryType>::size() const -> std::size_t
{
    return mPattern.size();
}

template <typename IndexType, typename EntryType>
auto SparseMatrixBuilder<IndexType, EntryType>::triple(const std::size_t aIndex) const
    -> std::tuple<Row, Column, EntryType>
{
    const auto tRowAndColumn = mPattern.rowAndColumn(aIndex);
    return std::make_tuple(tRowAndColumn.first, tRowAndColumn.second, mEntries[aIndex]);
}

template <typename IndexType, typename EntryType>
auto SparseMatrixBuilder<IndexType, EntryType>::rowData() -> NonNullPtr<IndexType>
{
    return mPattern.rowData();
}

template <typename IndexType, typename EntryType>
auto SparseMatrixBuilder<IndexType, EntryType>::columnData() -> NonNullPtr<IndexType>
{
    return mPattern.columnData();
}

template <typename IndexType, typename EntryType>
auto SparseMatrixBuilder<IndexType, EntryType>::entryData() -> NonNullPtr<EntryType>
{
    return NonNullPtr{mEntries.data()};
}

}  // namespace plato::third_party_integration::snopt

#endif
