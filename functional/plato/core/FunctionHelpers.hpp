#ifndef PLATO_CORE_FUNCTIONHELPERS
#define PLATO_CORE_FUNCTIONHELPERS

#include <functional>
#include <tuple>

#include "plato/utilities/TupleHelpers.hpp"

namespace plato::core
{
/// @brief Used for choosing a matrix ordering for matrix operations such as Jacobian-vector products.
enum struct MatrixOrdering
{
    kOriginal,
    kAdjoint
};

/// @brief Contains information needed to specify a function within Function.
/// @tparam CodomainType The return type of the function/function derivative.
/// @tparam kOrder The derivative order (mainly 0, 1, or 2).
/// @tparam kOrdering The matrix ordering of any matrix operations, such as the Jacobian.
template <typename CodomainType, int kOrder, MatrixOrdering kOrdering = MatrixOrdering::kOriginal>
struct FunctionInfo
{
    using Codomain = CodomainType;
    constexpr static int order = kOrder;
    constexpr static MatrixOrdering ordering = kOrdering;
};

namespace detail
{
/// @brief A helper for Function, matches function derivative with an index into the tuple of functions.
template <typename Domain, typename... Info>
struct MakeTupleHelper
{
    using FunctionTuple = std::tuple<std::function<typename Info::Codomain(Domain)>...>;
    using InfoTuple = std::tuple<Info...>;

    template <int kOrder, MatrixOrdering kOrdering, std::size_t kIndex>
    [[nodiscard]] static constexpr auto get_index_impl() -> std::size_t;

    template <int kOrder, MatrixOrdering kOrdering, std::size_t... kIndices>
    [[nodiscard]] static constexpr auto get_index(std::index_sequence<kIndices...>) -> std::size_t;

    template <int kOrder, MatrixOrdering kOrdering>
    [[nodiscard]] static constexpr auto index() -> std::size_t;

    template <int kOrder, MatrixOrdering kOrdering, std::size_t kIndex>
    [[nodiscard]] static constexpr auto is_implemented_impl() -> bool;

    template <int kOrder, MatrixOrdering kOrdering, std::size_t... kIndices>
    [[nodiscard]] static constexpr auto is_implemented_seq(std::index_sequence<kIndices...>) -> bool;

    template <int kOrder, MatrixOrdering kOrdering>
    [[nodiscard]] static constexpr auto is_implemented() -> bool;
};

/// @brief Returns `true` if @a TList has size 1 and the only type contained in it is @a T.
template <typename T, typename... TList>
constexpr auto is_only_member()
{
    if constexpr (std::tuple_size_v<std::tuple<TList...>> == 1)
    {
        return std::is_same_v<T, std::decay_t<std::tuple_element_t<0, std::tuple<TList...>>>>;
    }
    else
    {
        return false;
    }
}

/// @brief Returns `true` if a Function templated with arguments @a DomainTypeLHS and @a InfoTupleLHS can be copied
/// to from a Function templated with @a DomainTypeRHS and @a InfoTupleRHS, such that the LHS type implements a subset
/// of the functions implemented by the RHS.
///
/// More specifically, the following requirements must be met to by copyable:
///  * The domain types must be the same
///  * The info types in tuple @a InfoTupleLHS must be a subset of the types in the tuple @a InfoTupleRHS
///  * To match, an info object must match exactly: The Codomain type must be the same as well as order and ordering.
template <typename DomainTypeLHS, typename InfoTupleLHS, typename DomainTypeRHS, typename InfoTupleRHS>
constexpr auto is_copyable() -> bool;

template <typename Domain, typename... Info>
template <int kOrder, MatrixOrdering kOrdering, std::size_t kIndex>
constexpr auto MakeTupleHelper<Domain, Info...>::get_index_impl() -> std::size_t
{
    if constexpr (is_implemented_impl<kOrder, kOrdering, kIndex>())
    {
        return kIndex;
    }
    else
    {
        return 0;
    }
}

template <typename Domain, typename... Info>
template <int kOrder, MatrixOrdering kOrdering, std::size_t... kIndices>
constexpr auto MakeTupleHelper<Domain, Info...>::get_index(std::index_sequence<kIndices...>) -> std::size_t
{
    return (get_index_impl<kOrder, kOrdering, kIndices>() + ...);
}

template <typename Domain, typename... Info>
template <int kOrder, MatrixOrdering kOrdering>
constexpr auto MakeTupleHelper<Domain, Info...>::index() -> std::size_t
{
    return get_index<kOrder, kOrdering>(std::make_index_sequence<std::tuple_size_v<InfoTuple>>());
}

template <typename Domain, typename... Info>
template <int kOrder, MatrixOrdering kOrdering, std::size_t kIndex>
constexpr auto MakeTupleHelper<Domain, Info...>::is_implemented_impl() -> bool
{
    using InfoAtIndex = std::tuple_element_t<kIndex, InfoTuple>;
    return InfoAtIndex::order == kOrder && InfoAtIndex::ordering == kOrdering;
}

template <typename Domain, typename... Info>
template <int kOrder, MatrixOrdering kOrdering, std::size_t... kIndices>
constexpr auto MakeTupleHelper<Domain, Info...>::is_implemented_seq(std::index_sequence<kIndices...>) -> bool
{
    return (is_implemented_impl<kOrder, kOrdering, kIndices>() || ...);
}

template <typename Domain, typename... Info>
template <int kOrder, MatrixOrdering kOrdering>
constexpr auto MakeTupleHelper<Domain, Info...>::is_implemented() -> bool
{
    return is_implemented_seq<kOrder, kOrdering>(std::make_index_sequence<std::tuple_size_v<InfoTuple>>());
}

template <typename DomainTypeLHS, typename InfoTupleLHS, typename DomainTypeRHS, typename InfoTupleRHS>
constexpr auto is_copyable() -> bool
{
    constexpr auto tDomainTypesEqual = std::is_same_v<DomainTypeLHS, DomainTypeRHS>;
    constexpr auto tInfoTuplesAreCompatible = utilities::are_tuples_compatible<InfoTupleLHS, InfoTupleRHS>();
    return tDomainTypesEqual && tInfoTuplesAreCompatible;
}

}  // namespace detail

}  // namespace plato::core

#endif
