#ifndef PLATO_UTILITIES_ZIP
#define PLATO_UTILITIES_ZIP

#include <functional>
#include <iterator>
#include <tuple>
#include <utility>

#include "plato/utilities/ZipIterator.hpp"

namespace plato::utilities
{
/// @brief A helper class for iterating over multiple containers in range-based for loop.
///
/// An example of the usage is:
/// @code{.cpp}
/// const auto vector_1 = std::vector{1, 2, 3};
/// const auto vector_2 = std::vector{'a', 'b', 'c'};
/// for( const auto [ value_1, value_2 ] : Zip{ vector_1, vector_2 } )
/// { ... }
/// @endcode
///
/// If the containers have different sizes, the iteration stops at the smallest
/// of all the container sizes. Non-const containers may be modified and temporary
/// containers may also be used.
///
/// @note Temporaries are copied to ensure validity of the iterators.
/// @note The constness of the values in the structured binding declaration is determined by the
///  constness of the container, not the constness of the structured binding declaration. For
///  example, the following will compile and modify the values contained in `vector_1`:
/// @code{.cpp}
/// auto vector_1 = std::vector{1, 2, 3};
/// const auto vector_2 = std::vector{'a', 'b', 'c'};
/// for( const auto [ value_1, value_2 ] : Zip{ vector_1, vector_2 } )
/// { value_1 = 42; }
/// @endcode
template <typename... Containers>
class Zip
{
   public:
    template <typename... CtorContainers>
    constexpr explicit Zip(CtorContainers&&... containers);

    [[nodiscard]] constexpr auto begin();
    [[nodiscard]] constexpr auto begin() const;
    [[nodiscard]] constexpr auto end();
    [[nodiscard]] constexpr auto end() const;

   private:
    using ContainerTuple = std::tuple<Containers...>;
    ContainerTuple mContainers;
};

template <typename... Containers>
Zip(Containers&&...) -> Zip<Containers...>;

namespace detail
{
template <typename ContainerTuple, std::size_t... Is>
constexpr auto begin_iterators(ContainerTuple&& containers, std::index_sequence<Is...>)
{
    using std::begin;
    return std::make_tuple(begin(std::get<Is>(std::forward<ContainerTuple>(containers)))...);
}

template <typename ContainerTuple, std::size_t... Is>
constexpr auto const_begin_iterators(ContainerTuple&& containers, std::index_sequence<Is...>)
{
    using std::cbegin;
    return std::make_tuple(cbegin(std::get<Is>(std::forward<ContainerTuple>(containers)))...);
}

template <typename ContainerTuple, std::size_t... Is>
constexpr auto end_iterators(ContainerTuple&& containers, std::index_sequence<Is...>)
{
    using std::end;
    return std::make_tuple(end(std::get<Is>(std::forward<ContainerTuple>(containers)))...);
}

template <typename ContainerTuple, std::size_t... Is>
constexpr auto const_end_iterators(ContainerTuple&& containers, std::index_sequence<Is...>)
{
    using std::cend;
    return std::make_tuple(cend(std::get<Is>(std::forward<ContainerTuple>(containers)))...);
}

}  // namespace detail

template <typename... Containers>
template <typename... CtorContainers>
constexpr Zip<Containers...>::Zip(CtorContainers&&... containers) : mContainers{std::forward_as_tuple(containers...)}
{
}

template <typename... Containers>
constexpr auto Zip<Containers...>::begin()
{
    return ZipIterator{
        detail::begin_iterators(mContainers, std::make_index_sequence<std::tuple_size_v<ContainerTuple> >())};
}

template <typename... Containers>
constexpr auto Zip<Containers...>::begin() const
{
    return ZipIterator{
        detail::const_begin_iterators(mContainers, std::make_index_sequence<std::tuple_size_v<ContainerTuple> >())};
}

template <typename... Containers>
constexpr auto Zip<Containers...>::end()
{
    return ZipIterator{
        detail::end_iterators(mContainers, std::make_index_sequence<std::tuple_size_v<ContainerTuple> >())};
}

template <typename... Containers>
constexpr auto Zip<Containers...>::end() const
{
    return ZipIterator{
        detail::const_end_iterators(mContainers, std::make_index_sequence<std::tuple_size_v<ContainerTuple> >())};
}

}  // namespace plato::utilities

#endif
