#ifndef PLATO_UTILITIES_TUPLEHELPERS
#define PLATO_UTILITIES_TUPLEHELPERS

#include <tuple>

namespace plato::utilities
{
/// @brief Returns `true` if the types (which must be tuples) are compatible meaning that the types comprising @a
/// ToTuple are a subset of the types comprising @a FromTuple.
///
/// Here, a subset means that each type in @a ToTuple has an exact match in @a FromTuple by `std::is_same`.
/// @note This function is mainly usable for tuples consisting of unique types and a copy of only a subset of
///  @a FromTuple is needed, and so this will return `false` if either tuple does not have strictly unique types.
template <typename ToTuple, typename FromTuple>
[[nodiscard]] constexpr auto are_tuples_compatible();

/// @brief Copies or moves the entries of @a aFromTuple into a new tuple that may have types that are a subset of the
/// original tuple.
///
/// See are_tuples_compatible for the requirements on the types contained by the two tuples.
/// @sa are_tuples_compatible
template <typename ToTuple,
          typename FromTuple,
          typename = std::enable_if_t<are_tuples_compatible<ToTuple, std::decay_t<FromTuple>>()>>
[[nodiscard]] constexpr auto make_compatible_tuple(FromTuple&& aFromTuple) -> ToTuple;

namespace detail
{
template <typename T, typename Tuple, std::size_t kIndex>
[[nodiscard]] constexpr auto has_type_impl() -> bool
{
    if constexpr (kIndex < std::tuple_size_v<Tuple>)
    {
        return std::is_same_v<T, std::tuple_element_t<kIndex, Tuple>>;
    }
    else
    {
        return false;
    }
}

template <typename T, typename Tuple, std::size_t... kIndices>
[[nodiscard]] constexpr auto has_type(std::index_sequence<kIndices...>) -> bool
{
    return (has_type_impl<T, Tuple, kIndices>() || ...);
}

template <std::size_t kStart, std::size_t... kIndices>
[[nodiscard]] constexpr auto make_index_sequence_starting_from(std::index_sequence<kIndices...>)
{
    return std::index_sequence<kStart + kIndices...>{};
}

template <typename Tuple, std::size_t... kIndices>
[[nodiscard]] constexpr auto has_unique_types_seq(std::index_sequence<kIndices...>) -> bool
{
    return !(has_type<std::tuple_element_t<kIndices, Tuple>, Tuple>(make_index_sequence_starting_from<kIndices + 1>(
                 std::make_index_sequence<sizeof...(kIndices) - kIndices>())) ||
             ...);
}

template <typename Tuple>
[[nodiscard]] constexpr auto has_unique_types() -> bool
{
    return has_unique_types_seq<Tuple>(std::make_index_sequence<std::tuple_size_v<Tuple>>());
}

template <typename ToTuple, typename FromTuple, std::size_t... kLHSIndices>
[[nodiscard]] constexpr auto tuple_is_subset_seq(std::index_sequence<kLHSIndices...>) -> bool
{
    return (has_type<std::tuple_element_t<kLHSIndices, ToTuple>, FromTuple>(
                std::make_index_sequence<std::tuple_size_v<FromTuple>>()) &&
            ...);
}

template <typename ToTuple, typename FromTuple>
[[nodiscard]] constexpr auto tuple_is_subset() -> bool
{
    return tuple_is_subset_seq<ToTuple, FromTuple>(std::make_index_sequence<std::tuple_size_v<ToTuple>>());
}

template <std::size_t kToTupleIndex, typename ToTuple, typename FromTuple>
constexpr void assign_matching_rhs_element(ToTuple& aToTuple, FromTuple&& aFromTuple)
{
    std::get<kToTupleIndex>(aToTuple) =
        std::get<std::tuple_element_t<kToTupleIndex, ToTuple>>(std::forward<FromTuple>(aFromTuple));
}

template <typename ToTuple, typename FromTuple, std::size_t... kLHSIndices>
constexpr auto make_compatible_tuple_seq(FromTuple&& aFromTuple, std::index_sequence<kLHSIndices...>)
{
    auto tToTuple = ToTuple{};
    (assign_matching_rhs_element<kLHSIndices>(tToTuple, std::forward<FromTuple>(aFromTuple)), ...);
    return tToTuple;
}

}  // namespace detail

template <typename ToTuple, typename FromTuple>
constexpr auto are_tuples_compatible()
{
    constexpr auto tSizesCorrect = std::tuple_size_v<ToTuple> <= std::tuple_size_v<FromTuple>;
    constexpr auto tToTupleHasUniqueTypes = detail::has_unique_types<ToTuple>();
    constexpr auto tFromTupleHasUniqueTypes = detail::has_unique_types<FromTuple>();
    if constexpr (tSizesCorrect && tToTupleHasUniqueTypes && tFromTupleHasUniqueTypes)
    {
        return detail::tuple_is_subset<ToTuple, FromTuple>();
    }
    return false;
}

template <typename ToTuple, typename FromTuple, typename>
constexpr auto make_compatible_tuple(FromTuple&& aFromTuple) -> ToTuple
{
    return detail::make_compatible_tuple_seq<ToTuple, FromTuple>(
        std::forward<FromTuple>(aFromTuple), std::make_index_sequence<std::tuple_size_v<ToTuple>>());
}

}  // namespace plato::utilities

#endif
