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
    using pointer = const value_type*;
    using reference = const value_type&;

    constexpr MultidimensionalIterator(std::tuple<Ts...> aDimensions);
    constexpr MultidimensionalIterator(EndTag /*unused*/, std::tuple<Ts...> aDimensions);

    [[nodiscard]] constexpr auto operator*() const -> reference;
    constexpr auto operator++() -> MultidimensionalIterator&;

    [[nodiscard]] constexpr auto operator!=(const MultidimensionalIterator& aIterator) const noexcept -> bool;
    [[nodiscard]] constexpr auto operator==(const MultidimensionalIterator& aIterator) const noexcept -> bool;

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
    constexpr MultidimensionalRange(Ts... aDimensions);

    [[nodiscard]] constexpr auto begin() const -> MultidimensionalIterator<Ts...>;
    [[nodiscard]] constexpr auto end() const -> MultidimensionalIterator<Ts...>;

   private:
    std::tuple<Ts...> mDimensions{};
};

/// @brief Checks that all entries in tuples @a t1 and @a t2 satisfy the binary predicate @a op.
template <typename Tuple, typename BinaryOp>
constexpr auto tuple_all_of(const Tuple& aT1, const Tuple& aT2, const BinaryOp& aOp) -> bool;

namespace detail
{
template <std::size_t kPlace, typename Tuple>
void increment_and_carry(Tuple& aCount, const Tuple& aDimensions)
{
    auto& tValue = std::get<kPlace>(aCount);
    ++tValue;
    if constexpr (kPlace > 0)
    {
        if (tValue == std::get<kPlace>(aDimensions))
        {
            tValue = std::tuple_element_t<kPlace, Tuple>{0};
            increment_and_carry<kPlace - 1>(aCount, aDimensions);
        }
    }
}

template <typename Tuple, typename BinaryOp, std::size_t... kIs>
constexpr auto all_of_impl(const Tuple& aT1,
                           const Tuple& aT2,
                           const BinaryOp& aOp,
                           std::index_sequence<kIs...> /*unused*/) -> bool
{
    return (aOp(std::get<kIs>(aT1), std::get<kIs>(aT2)) && ...);
}
}  // namespace detail

template <typename Tuple, typename BinaryOp>
constexpr auto tuple_all_of(const Tuple& aT1, const Tuple& aT2, const BinaryOp& aOp) -> bool
{
    return detail::all_of_impl(aT1, aT2, aOp, std::make_index_sequence<std::tuple_size_v<Tuple> >());
}

template <typename... Ts>
constexpr MultidimensionalIterator<Ts...>::MultidimensionalIterator(const std::tuple<Ts...> aDimensions)
    : mDimensions{std::move(aDimensions)}
{
}

template <typename... Ts>
constexpr MultidimensionalIterator<Ts...>::MultidimensionalIterator(EndTag /*unused*/,
                                                                    const std::tuple<Ts...> aDimensions)
    : mDimensions{std::move(aDimensions)}
{
    std::get<0>(mCounts) = std::get<0>(mDimensions);
}

template <typename... Ts>
constexpr auto MultidimensionalIterator<Ts...>::operator*() const -> MultidimensionalIterator<Ts...>::reference
{
    return mCounts;
}

template <typename... Ts>
constexpr auto MultidimensionalIterator<Ts...>::operator++() -> MultidimensionalIterator<Ts...>&
{
    constexpr auto tNumRanks = std::tuple_size_v<value_type>;
    detail::increment_and_carry<tNumRanks - 1>(mCounts, mDimensions);
    return *this;
}

template <typename... Ts>
constexpr auto MultidimensionalIterator<Ts...>::operator!=(const MultidimensionalIterator& aIterator) const noexcept
    -> bool
{
    return mCounts != aIterator.mCounts;
}

template <typename... Ts>
constexpr auto MultidimensionalIterator<Ts...>::operator==(const MultidimensionalIterator& aIterator) const noexcept
    -> bool
{
    return mCounts == aIterator.mCounts;
}

template <typename... Ts>
constexpr MultidimensionalRange<Ts...>::MultidimensionalRange(Ts... aDimensions) : mDimensions{aDimensions...}
{
    constexpr auto tNumRanks = sizeof...(Ts);
    static_assert(tNumRanks > 0, "MultidimensionalRange must have at least one dimension.");

    constexpr auto tAllIntegerLike = std::conjunction_v<std::is_integral<Ts>...>;
    static_assert(tAllIntegerLike, "MultidimensionalRange must be instantiated with integral types.");

    // Check that all are greater than 0
    [[maybe_unused]] const bool tAllDimsGreaterThanCount =
        tuple_all_of(mDimensions, std::tuple<Ts...>{}, std::greater{});
    assert(tAllDimsGreaterThanCount);
}

template <typename... Ts>
constexpr auto MultidimensionalRange<Ts...>::begin() const -> MultidimensionalIterator<Ts...>
{
    return MultidimensionalIterator<Ts...>{mDimensions};
}

template <typename... Ts>
constexpr auto MultidimensionalRange<Ts...>::end() const -> MultidimensionalIterator<Ts...>
{
    return MultidimensionalIterator<Ts...>{typename MultidimensionalIterator<Ts...>::EndTag{}, mDimensions};
}

}  // namespace plato::utilities

#endif
