#ifndef PLATO_UTILITIES_ZIPITERATOR
#define PLATO_UTILITIES_ZIPITERATOR

#include <functional>
#include <iterator>
#include <tuple>
#include <utility>

namespace plato::utilities
{
/// @brief An iterator type for iterating over multiple ranges, mainly a helper
/// for Zip, though this can be used in standard algorithms as well.
/// @sa Zip
template <typename... Iterators>
class ZipIterator
{
   public:
    using iterator_category = std::input_iterator_tag;
    using value_type = std::tuple<typename std::iterator_traits<Iterators>::value_type...>;
    using reference = std::tuple<typename std::iterator_traits<Iterators>::reference...>;
    using const_reference = std::tuple<const typename std::iterator_traits<Iterators>::reference...>;

    constexpr explicit ZipIterator(const Iterators&... aIterators);
    constexpr explicit ZipIterator(std::tuple<Iterators...> aIterators);

    [[nodiscard]] constexpr auto operator*() -> reference;
    [[nodiscard]] constexpr auto operator*() const -> const_reference;
    constexpr auto operator++() -> ZipIterator&;

    [[nodiscard]] constexpr auto operator!=(const ZipIterator& aIterator) const noexcept -> bool;
    [[nodiscard]] constexpr auto operator==(const ZipIterator& aIterator) const noexcept -> bool;

   private:
    std::tuple<Iterators...> mIterators{};
};

namespace detail
{
template <typename Tuple, typename BinaryOp, std::size_t... kIs>
constexpr auto any_of_comparison_impl(const Tuple& aT1,
                                      const Tuple& aT2,
                                      const BinaryOp& aOp,
                                      std::index_sequence<kIs...> /*unused*/) -> bool
{
    return (aOp(std::get<kIs>(aT1), std::get<kIs>(aT2)) || ...);
}

template <typename ReferenceTuple, typename IteratorTuple, std::size_t... kIs>
constexpr auto dereference(const IteratorTuple& aIteratorTuple, std::index_sequence<kIs...> /*unused*/)
    -> decltype(auto)
{
    return ReferenceTuple(*std::get<kIs>(aIteratorTuple)...);
}

template <typename ReferenceTuple, typename IteratorTuple, std::size_t... kIs>
constexpr auto const_dereference(const IteratorTuple& aIteratorTuple, std::index_sequence<kIs...> /*unused*/)
    -> decltype(auto)
{
    return ReferenceTuple(std::as_const(*std::get<kIs>(aIteratorTuple))...);
}

template <typename IteratorTuple, std::size_t... kIs>
constexpr void increment(IteratorTuple& aIteratorTuple, std::index_sequence<kIs...> /*unused*/)
{
    (++std::get<kIs>(aIteratorTuple), ...);
}
}  // namespace detail

template <typename Tuple, typename BinaryOp>
constexpr auto any_of_comparison(const Tuple& aT1, const Tuple& aT2, const BinaryOp& aOp) -> bool
{
    return detail::any_of_comparison_impl(aT1, aT2, aOp, std::make_index_sequence<std::tuple_size_v<Tuple> >());
}

template <typename... Iterators>
constexpr ZipIterator<Iterators...>::ZipIterator(const Iterators&... aIterators) : mIterators{aIterators...}
{
}

template <typename... Iterators>
constexpr ZipIterator<Iterators...>::ZipIterator(std::tuple<Iterators...> aIterators)
    : mIterators{std::move(aIterators)}
{
}

template <typename... Iterators>
constexpr auto ZipIterator<Iterators...>::operator*() -> ZipIterator<Iterators...>::reference
{
    return detail::dereference<reference>(mIterators, std::make_index_sequence<std::tuple_size_v<value_type> >());
}

template <typename... Iterators>
constexpr auto ZipIterator<Iterators...>::operator*() const -> ZipIterator<Iterators...>::const_reference
{
    return detail::const_dereference<const_reference>(mIterators,
                                                      std::make_index_sequence<std::tuple_size_v<value_type> >());
}

template <typename... Iterators>
constexpr auto ZipIterator<Iterators...>::operator++() -> ZipIterator<Iterators...>&
{
    detail::increment(mIterators, std::make_index_sequence<std::tuple_size_v<value_type> >());
    return *this;
}

template <typename... Iterators>
constexpr auto ZipIterator<Iterators...>::operator!=(const ZipIterator<Iterators...>& aIterator) const noexcept -> bool
{
    return !any_of_comparison(mIterators, aIterator.mIterators, std::equal_to{});
}

template <typename... Iterators>
constexpr auto ZipIterator<Iterators...>::operator==(const ZipIterator<Iterators...>& aIterator) const noexcept -> bool
{
    return any_of_comparison(mIterators, aIterator.mIterators, std::equal_to{});
}

}  // namespace plato::utilities

#endif
