#ifndef PLATO_UTILITIES_MULTIDIMENSIOALRANGE_HPP
#define PLATO_UTILITIES_MULTIDIMENSIOALRANGE_HPP

#include <cassert>
#include <cstddef>
#include <functional>
#include <tuple>
#include <utility>

namespace plato::utilities
{
/// @brief Iterator used with MultidimensionalRange.
template <typename... Ts>
class MultidimensionalIterator
{
   public:
    struct EndTag
    {
    };

    using iterator_category = std::input_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using value_type = std::tuple<Ts...>;
    using pointer = MultidimensionalIterator*;
    using reference = MultidimensionalIterator&;

    constexpr MultidimensionalIterator(const std::tuple<Ts...> dimensions);
    constexpr MultidimensionalIterator(EndTag, const std::tuple<Ts...> dimensions);

    [[nodiscard]] constexpr value_type operator*() const;
    constexpr MultidimensionalIterator& operator++();

    [[nodiscard]] constexpr bool operator!=(const MultidimensionalIterator& iterator) const noexcept;
    [[nodiscard]] constexpr bool operator==(const MultidimensionalIterator& iterator) const noexcept;

   private:
    std::tuple<Ts...> mCounts{};
    std::tuple<Ts...> mDimensions{};
};

/// @brief Used for iterating over multiple dimensions, e.g. to collapse nested for loops.
///
/// This can be used in range-based for loops as:
/// @code{.cpp}
/// for( const auto [index_0, index_1, index_2] : MultidimensionalRange{ 4, 5, 6 } ) { ... }
/// @endcode
/// This iterates through all values of the indices, resulting in 120 (`4*5*6`) iterations for this example.
/// Iteration always starts at 0 for each index and iterates over the right-most index first. I.e.,
/// in the above example, the values of the indices will be:
/// @code
/// 0, 0, 0
/// 0, 0, 1
/// ...
/// 0, 0, 5
/// 0, 1, 0
/// ...
/// @endcode
///
/// @todo Replace `typename` with `std::integral` concept in c++20.
template <typename... Ts>
class MultidimensionalRange
{
   public:
    /// @pre All values of @a dimensions must greater than 0.
    constexpr MultidimensionalRange(Ts... dimensions);

    [[nodiscard]] constexpr MultidimensionalIterator<Ts...> begin() const;
    [[nodiscard]] constexpr MultidimensionalIterator<Ts...> end() const;

   private:
    std::tuple<Ts...> mDimensions{};
};

/// @brief Checks that all entries in tuples @a t1 and @a t2 satisfy the binary predicate @a op.
template <typename Tuple, typename BinaryOp>
constexpr bool allOf(const Tuple& t1, const Tuple& t2, const BinaryOp& op);

namespace detail
{
template <std::size_t Place, typename Tuple>
void incrementAndCarry(Tuple& count, const Tuple& dimensions)
{
    auto& tValue = std::get<Place>(count);
    ++tValue;
    if constexpr (Place > 0)
    {
        if (tValue == std::get<Place>(dimensions))
        {
            tValue = std::tuple_element_t<Place, Tuple>{0};
            incrementAndCarry<Place - 1>(count, dimensions);
        }
    }
}

template <typename Tuple, typename BinaryOp, std::size_t... Is>
constexpr bool allOfImpl(const Tuple& t1, const Tuple& t2, const BinaryOp& op, std::index_sequence<Is...>)
{
    return (op(std::get<Is>(t1), std::get<Is>(t2)) && ...);
}
}  // namespace detail

template <typename Tuple, typename BinaryOp>
constexpr bool allOf(const Tuple& t1, const Tuple& t2, const BinaryOp& op)
{
    return detail::allOfImpl(t1, t2, op, std::make_index_sequence<std::tuple_size_v<Tuple> >());
}

template <typename... Ts>
constexpr MultidimensionalIterator<Ts...>::MultidimensionalIterator(const std::tuple<Ts...> dimensions)
    : mCounts{}, mDimensions{dimensions}
{
}

template <typename... Ts>
constexpr MultidimensionalIterator<Ts...>::MultidimensionalIterator(EndTag, const std::tuple<Ts...> dimensions)
    : mCounts{}, mDimensions{dimensions}
{
    std::get<0>(mCounts) = std::get<0>(mDimensions);
}

template <typename... Ts>
constexpr auto MultidimensionalIterator<Ts...>::operator*() const -> MultidimensionalIterator<Ts...>::value_type
{
    return mCounts;
}

template <typename... Ts>
constexpr auto MultidimensionalIterator<Ts...>::operator++() -> MultidimensionalIterator<Ts...>&
{
    constexpr auto num_ranks = std::tuple_size_v<value_type>;
    detail::incrementAndCarry<num_ranks - 1>(mCounts, mDimensions);
    return *this;
}

template <typename... Ts>
constexpr bool MultidimensionalIterator<Ts...>::operator!=(const MultidimensionalIterator& iterator) const noexcept
{
    return mCounts != iterator.mCounts;
}

template <typename... Ts>
constexpr bool MultidimensionalIterator<Ts...>::operator==(const MultidimensionalIterator& iterator) const noexcept
{
    return mCounts == iterator.mCounts;
}

template <typename... Ts>
constexpr MultidimensionalRange<Ts...>::MultidimensionalRange(Ts... dimensions) : mDimensions{dimensions...}
{
    constexpr auto num_ranks = sizeof...(Ts);
    static_assert(num_ranks > 0, "MultidimensionalRange must have at least one dimension.");

    constexpr auto all_integer_like = std::conjunction_v<std::is_integral<Ts>...>;
    static_assert(all_integer_like, "MultidimensionalRange must be instantiated with integral types.");

    // Check that all are greater than 0
    [[maybe_unused]] const bool all_dims_greater_than_count = allOf(mDimensions, std::tuple<Ts...>{}, std::greater{});
    assert(all_dims_greater_than_count);
}

template <typename... Ts>
constexpr MultidimensionalIterator<Ts...> MultidimensionalRange<Ts...>::begin() const
{
    return MultidimensionalIterator<Ts...>{mDimensions};
}

template <typename... Ts>
constexpr MultidimensionalIterator<Ts...> MultidimensionalRange<Ts...>::end() const
{
    return MultidimensionalIterator<Ts...>{typename MultidimensionalIterator<Ts...>::EndTag{}, mDimensions};
}

}  // namespace plato::utilities

#endif
