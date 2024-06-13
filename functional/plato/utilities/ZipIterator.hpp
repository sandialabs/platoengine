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

    constexpr explicit ZipIterator(const Iterators&... iterators);
    constexpr explicit ZipIterator(std::tuple<Iterators...> iterators);

    [[nodiscard]] constexpr reference operator*();
    [[nodiscard]] constexpr auto operator*() const;
    constexpr ZipIterator& operator++();

    [[nodiscard]] constexpr bool operator!=(const ZipIterator& iterator) const noexcept;
    [[nodiscard]] constexpr bool operator==(const ZipIterator& iterator) const noexcept;

   private:
    std::tuple<Iterators...> mIterators{};
};

namespace detail
{
template <typename Tuple, typename BinaryOp, std::size_t... Is>
constexpr bool any_of_comparison_impl(const Tuple& t1, const Tuple& t2, const BinaryOp& op, std::index_sequence<Is...>)
{
    return (op(std::get<Is>(t1), std::get<Is>(t2)) || ...);
}

template <typename IteratorTuple, std::size_t... Is>
constexpr auto dereference(const IteratorTuple& iterator_tuple, std::index_sequence<Is...>)
{
    return std::forward_as_tuple(*std::get<Is>(iterator_tuple)...);
}

template <typename IteratorTuple, std::size_t... Is>
constexpr auto const_dereference(const IteratorTuple& iterator_tuple, std::index_sequence<Is...>)
{
    return std::forward_as_tuple(std::as_const(*std::get<Is>(iterator_tuple))...);
}

template <typename IteratorTuple, std::size_t... Is>
constexpr void increment(IteratorTuple& iterator_tuple, std::index_sequence<Is...>)
{
    (++std::get<Is>(iterator_tuple), ...);
}
}  // namespace detail

template <typename Tuple, typename BinaryOp>
constexpr bool any_of_comparison(const Tuple& t1, const Tuple& t2, const BinaryOp& op)
{
    return detail::any_of_comparison_impl(t1, t2, op, std::make_index_sequence<std::tuple_size_v<Tuple> >());
}

template <typename... Iterators>
constexpr ZipIterator<Iterators...>::ZipIterator(const Iterators&... iterators) : mIterators{iterators...}
{
}

template <typename... Iterators>
constexpr ZipIterator<Iterators...>::ZipIterator(std::tuple<Iterators...> iterators) : mIterators{std::move(iterators)}
{
}

template <typename... Iterators>
constexpr auto ZipIterator<Iterators...>::operator*() -> ZipIterator<Iterators...>::reference
{
    return detail::dereference(mIterators, std::make_index_sequence<std::tuple_size_v<value_type> >());
}

template <typename... Iterators>
constexpr auto ZipIterator<Iterators...>::operator*() const
{
    return detail::const_dereference(mIterators, std::make_index_sequence<std::tuple_size_v<value_type> >());
}

template <typename... Iterators>
constexpr ZipIterator<Iterators...>& ZipIterator<Iterators...>::operator++()
{
    detail::increment(mIterators, std::make_index_sequence<std::tuple_size_v<value_type> >());
    return *this;
}

template <typename... Iterators>
constexpr bool ZipIterator<Iterators...>::operator!=(const ZipIterator<Iterators...>& iterator) const noexcept
{
    return !any_of_comparison(mIterators, iterator.mIterators, std::equal_to{});
}

template <typename... Iterators>
constexpr bool ZipIterator<Iterators...>::operator==(const ZipIterator<Iterators...>& iterator) const noexcept
{
    return any_of_comparison(mIterators, iterator.mIterators, std::equal_to{});
}

}  // namespace plato::utilities

#endif
