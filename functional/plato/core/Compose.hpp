#ifndef PLATO_CORE_COMPOSE
#define PLATO_CORE_COMPOSE

#include <type_traits>
#include <utility>

#include "plato/core/Function.hpp"

namespace plato::core
{
/// @brief Generates a new Function that is the composition of @a aFunctionOuter and @a aFunctionInner, i.e.
/// \f$f(g(x))\f$
/// @param aFunctionOuter The outer function, \f$f\f$ in \f$f(g(x))\f$
/// @param aFunctionInner The inner function, \f$g\f$ in \f$f(g(x))\f$
template <typename FunctionTypeOuter, typename FunctionTypeInner>
auto compose(const FunctionTypeOuter& aFunctionOuter, const FunctionTypeInner& aFunctionInner);

namespace detail
{
template <std::size_t kIndex, typename Tuple, typename FunctionTypeOuter, typename FunctionTypeInner>
auto compose_impl(Tuple&& aTuple, FunctionTypeOuter aFunctionOuter, FunctionTypeInner aFunctionInner)
{
    using InfoAtIndex = std::tuple_element_t<kIndex, typename FunctionTypeOuter::TupleHelper::InfoTuple>;
    using Domain = typename FunctionTypeInner::Domain;
    if constexpr (InfoAtIndex::order == 0)
    {
        auto tFOfG = [tF = std::move(aFunctionOuter), tG = std::move(aFunctionInner)](Domain aX)
        { return tF.template evaluate<0>(tG.template evaluate<0>(aX)); };
        return std::tuple_cat(std::forward<Tuple>(aTuple), std::make_tuple(std::move(tFOfG)));
    }
    else if constexpr (InfoAtIndex::order == 1 && InfoAtIndex::ordering == MatrixOrdering::kOriginal)
    {
        auto tDfOfG = [tF = std::move(aFunctionOuter), tG = std::move(aFunctionInner)](Domain aX)
        { return tF.template evaluate<1>(tG.template evaluate<0>(aX)) * tG.template evaluate<1>(aX); };
        return std::tuple_cat(std::forward<Tuple>(aTuple), std::make_tuple(std::move(tDfOfG)));
    }
    else if constexpr (InfoAtIndex::order == 1 && InfoAtIndex::ordering == MatrixOrdering::kAdjoint)
    {
        auto tDfOfG = [tF = std::move(aFunctionOuter), tG = std::move(aFunctionInner)](Domain aX)
        {
            return tG.template evaluate<1, MatrixOrdering::kAdjoint>(aX) *
                   tF.template evaluate<1, MatrixOrdering::kAdjoint>(tG.template evaluate<0>(aX));
        };
        return std::tuple_cat(std::forward<Tuple>(aTuple), std::make_tuple(std::move(tDfOfG)));
    }
    else
    {
        static_assert(InfoAtIndex::order < 2, "Higher-order derivatives not yet implemented in compose.");
        return std::forward<Tuple>(aTuple);
    }
}

template <std::size_t kIndex, typename Tuple, typename FunctionTypeOuter, typename FunctionTypeInner>
auto compose_sequence(Tuple&& aTuple, const FunctionTypeOuter& aFunctionOuter, const FunctionTypeInner& aFunctionInner)
{
    if constexpr (kIndex < std::tuple_size_v<typename FunctionTypeOuter::TupleHelper::InfoTuple>)
    {
        return compose_sequence<kIndex + 1>(
            compose_impl<kIndex>(std::forward<Tuple>(aTuple), aFunctionOuter, aFunctionInner), aFunctionOuter,
            aFunctionInner);
    }
    else
    {
        return std::forward<Tuple>(aTuple);
    }
}

template <std::size_t kIndex,
          typename Domain,
          typename FunctionCompositionTuple,
          typename InfoTuple,
          typename CurrentTuple>
auto info_tuple_with_composition_codomains_impl(CurrentTuple&& aCurrentTuple)
{
    if constexpr (kIndex < std::tuple_size_v<InfoTuple>)
    {
        using CodomainType = std::invoke_result_t<std::tuple_element_t<kIndex, FunctionCompositionTuple>, Domain>;
        using CurrentFunctionInfo = std::tuple_element_t<kIndex, InfoTuple>;
        using ComposedFunctionInfo =
            FunctionInfo<CodomainType, CurrentFunctionInfo::order, CurrentFunctionInfo::ordering>;
        return info_tuple_with_composition_codomains_impl<kIndex + 1, Domain, FunctionCompositionTuple, InfoTuple>(
            std::tuple_cat(std::forward<CurrentTuple>(aCurrentTuple), std::make_tuple(ComposedFunctionInfo{})));
    }
    else
    {
        return std::forward<CurrentTuple>(aCurrentTuple);
    }
}

template <typename Domain, typename FunctionCompositionTuple, typename InfoTuple>
auto info_tuple_with_composition_codomains()
{
    static_assert(std::tuple_size_v<FunctionCompositionTuple> == std::tuple_size_v<InfoTuple>,
                  "info_tuple_with_composition_codomains must be instantiated with tuples of the same size.");

    constexpr auto tStartIndex = std::size_t{0};
    return info_tuple_with_composition_codomains_impl<tStartIndex, Domain, FunctionCompositionTuple, InfoTuple>(
        std::tuple{});
}

template <typename Domain, typename InfoTuple, typename FunctionTuple, std::size_t... Indices>
auto function_from_info_tuple(std::index_sequence<Indices...>)
{
    using CompositionInfoTuple = decltype(info_tuple_with_composition_codomains<Domain, FunctionTuple, InfoTuple>());
    return Function<Domain, std::tuple_element_t<Indices, CompositionInfoTuple>...>{
        std::function<typename std::tuple_element_t<Indices, CompositionInfoTuple>::Codomain(Domain)>{}...};
}
}  // namespace detail

template <typename FunctionTypeOuter, typename FunctionTypeInner>
auto compose(const FunctionTypeOuter& aFunctionOuter, const FunctionTypeInner& aFunctionInner)
{
    static_assert(std::tuple_size_v<typename FunctionTypeOuter::TupleHelper::InfoTuple> ==
                      std::tuple_size_v<typename FunctionTypeInner::TupleHelper::InfoTuple>,
                  "aFunctionOuter and aFunctionInner must implement the same number of functions.");
    constexpr auto tStartingIndex = int{0};
    using OuterInfo = typename FunctionTypeOuter::TupleHelper::InfoTuple;
    using InnerDomain = typename FunctionTypeInner::Domain;

    auto tComposedFunctions =
        detail::compose_sequence<tStartingIndex>(std::make_tuple(), aFunctionOuter, aFunctionInner);

    using CompositionType =
        decltype(detail::function_from_info_tuple<InnerDomain, OuterInfo, decltype(tComposedFunctions)>(
            std::make_index_sequence<std::tuple_size_v<OuterInfo>>()));

    return std::make_from_tuple<CompositionType>(std::move(tComposedFunctions));
}

}  // namespace plato::core

#endif
