#ifndef PLATO_UTILITIES_ENUMINDEXING
#define PLATO_UTILITIES_ENUMINDEXING

#include <cstddef>
#include <tuple>
#include <utility>

namespace plato::utilities
{
/// @brief Returns a linear index associated with a set of ordered enums.
/// @note Each enum must contain enumerates starting from 0, increasing by 1. The last entry must be
/// `kNumberOfEnumerates`.
///
/// The linear index corresponds to the entry in a linear array as if it were arranged as an N-D array with dimensions
/// corresponding to the number of enums. For example if A is an enum with 2 entries and B is an enum with 3 entries,
/// `enum_index` represents a linear index into a 2x3 array:
///   `enum_index(A::kZero, B::kZero) == 0`,
///   `enum_index(A::kOne, B::kZero) == 1`,
///   `enum_index(A::kZero, B::kOne) == 2`,
///   `enum_index(A::kOne, B::kOne) == 3`,
///   `enum_index(A::kZero, B::kTwo) == 4`,
///   `enum_index(A::kOne, B::kTwo) == 5`.
template <typename... Enums>
[[nodiscard]] constexpr auto enum_index(const Enums... aEnums) -> std::size_t;

/// @brief The reverse of enum_index, this returns the set of enums associated with index @a aIndex.
/// @sa enum_index
template <typename... Enums>
[[nodiscard]] constexpr auto enums_from_index(std::size_t aIndex) -> std::tuple<Enums...>;

namespace detail
{
template <typename Tuple, std::size_t... kIndices>
[[nodiscard]] constexpr auto dimension_stride_impl(std::index_sequence<kIndices...>) -> std::size_t
{
    return (static_cast<std::size_t>(std::tuple_element_t<kIndices, Tuple>::kNumberOfEnumerates) * ...);
}

template <std::size_t kIndex, typename... Enums>
[[nodiscard]] constexpr auto dimension_stride() -> std::size_t
{
    if constexpr (kIndex == 0U)
    {
        return 1U;
    }
    else
    {
        return dimension_stride_impl<std::tuple<Enums...>>(std::make_index_sequence<kIndex>());
    }
}

template <typename EnumTuple, std::size_t... kIndices>
[[nodiscard]] constexpr auto enum_index_impl(const EnumTuple& aEnumTuple, std::index_sequence<kIndices...>)
{
    return ((static_cast<std::size_t>(std::get<kIndices>(aEnumTuple)) *
             dimension_stride<kIndices, std::tuple_element_t<kIndices, EnumTuple>...>()) +
            ...);
}

template <typename AllEnumsTuple, std::size_t kDimension>
[[nodiscard]] constexpr auto enums_from_index(const std::size_t aLinearIndex)
{
    using CurrentTuple = std::tuple_element_t<kDimension, AllEnumsTuple>;
    constexpr auto tDimensionSize = static_cast<std::size_t>(CurrentTuple::kNumberOfEnumerates);
    const auto tDimensionIndex = aLinearIndex % tDimensionSize;
    const auto tNextDimensionIndex = aLinearIndex / tDimensionSize;
    const auto tEnum = static_cast<CurrentTuple>(tDimensionIndex);
    if constexpr (kDimension + 1 == std::tuple_size_v<AllEnumsTuple>)
    {
        return std::make_tuple(tEnum);
    }
    else
    {
        return std::tuple_cat(std::make_tuple(tEnum),
                              enums_from_index<AllEnumsTuple, kDimension + 1>(tNextDimensionIndex));
    }
}
}  // namespace detail

template <typename... Enums>
[[nodiscard]] constexpr auto enum_index(const Enums... aEnums) -> std::size_t
{
    static_assert(sizeof...(Enums) > 0, "enum_index must be called with one or more enums.");
    static_assert(std::conjunction_v<std::is_enum<Enums>...>, "enum_index must only be called with enum arguments.");

    return detail::enum_index_impl(std::make_tuple(aEnums...), std::make_index_sequence<sizeof...(Enums)>());
}

template <typename... Enums>
[[nodiscard]] constexpr auto enums_from_index(const std::size_t aIndex) -> std::tuple<Enums...>
{
    static_assert(sizeof...(Enums) > 0, "enums_from_index must be called with one or more enums.");
    static_assert(std::conjunction_v<std::is_enum<Enums>...>,
                  "enums_from_index must only be called with enum template parameters.");

    using EnumTuple = std::tuple<Enums...>;
    constexpr auto tStartDimension = std::size_t{0U};
    return detail::enums_from_index<EnumTuple, tStartDimension>(aIndex);
}

}  // namespace plato::utilities

#endif
