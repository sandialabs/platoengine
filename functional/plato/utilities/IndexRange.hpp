#ifndef PLATO_UTILITIES_INDEXRANGE_HPP
#define PLATO_UTILITIES_INDEXRANGE_HPP

#include "plato/utilities/MultidimensionalRange.hpp"

namespace plato::utilities
{

/// @brief An iterator for iterating over an index range, useful for std algorithms.
template <typename T>
class IndexIterator
{
   public:
    struct EndTag
    {
    };

    using iterator_category = std::input_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using value_type = T;
    using pointer = const T*;
    using reference = const T&;

    constexpr IndexIterator(const T aDimensions);
    constexpr IndexIterator(EndTag, const T aDimensions);

    [[nodiscard]] constexpr reference operator*() const;
    constexpr IndexIterator& operator++();

    [[nodiscard]] constexpr bool operator!=(const IndexIterator& aIterator) const noexcept;
    [[nodiscard]] constexpr bool operator==(const IndexIterator& aIterator) const noexcept;

   private:
    MultidimensionalIterator<T> mIterator;
};

/// @brief Used for iterating over a single index in a range-based for loop.
///
/// This can be used in range-based for loops as:
/// @code{.cpp}
/// for( const auto index : IndexRange{ 4 } ) { ... }
/// @endcode
template <typename T>
class IndexRange
{
   public:
    /// @pre All values of @a dimensions must greater than 0.
    constexpr IndexRange(T aDimensions);

    [[nodiscard]] constexpr IndexIterator<T> begin() const;
    [[nodiscard]] constexpr IndexIterator<T> end() const;

   private:
    T mDimensions;
};

template <typename T>
constexpr IndexIterator<T>::IndexIterator(const T aDimensions) : mIterator{aDimensions}
{
}

template <typename T>
constexpr IndexIterator<T>::IndexIterator(EndTag, const T aDimensions)
    : mIterator{typename MultidimensionalIterator<T>::EndTag{}, aDimensions}
{
}

template <typename T>
constexpr auto IndexIterator<T>::operator*() const -> typename IndexIterator<T>::reference
{
    return std::get<0>(*mIterator);
}

template <typename T>
constexpr IndexIterator<T>& IndexIterator<T>::operator++()
{
    ++mIterator;
    return *this;
}

template <typename T>
constexpr bool IndexIterator<T>::operator!=(const IndexIterator<T>& aIterator) const noexcept
{
    return mIterator != aIterator.mIterator;
}

template <typename T>
constexpr bool IndexIterator<T>::operator==(const IndexIterator<T>& aIterator) const noexcept
{
    return mIterator == aIterator.mIterator;
}

template <typename T>
constexpr IndexRange<T>::IndexRange(T aDimensions) : mDimensions{aDimensions}
{
}

template <typename T>
constexpr IndexIterator<T> IndexRange<T>::begin() const
{
    return IndexIterator<T>{mDimensions};
}

template <typename T>
constexpr IndexIterator<T> IndexRange<T>::end() const
{
    return IndexIterator<T>{typename IndexIterator<T>::EndTag{}, mDimensions};
}

}  // namespace plato::utilities

#endif
