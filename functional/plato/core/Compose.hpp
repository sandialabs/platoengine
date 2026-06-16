#ifndef PLATO_CORE_COMPOSE
#define PLATO_CORE_COMPOSE

#include <type_traits>
#include <utility>

#include "plato/core/Function.hpp"

namespace plato::core
{
namespace detail
{
// NOLINTBEGIN(misc-redundant-expression)
/// @brief Requirement for the number of functions implemented in the outer and inner functions for compose.
template <typename FunctionTypeOuter, typename FunctionTypeInner>
constexpr bool kFunctionCompositionSizesRequirement =
    std::tuple_size_v<typename FunctionTypeOuter::TupleHelper::InfoTuple> <=
    std::tuple_size_v<typename FunctionTypeInner::TupleHelper::InfoTuple>;
// NOLINTEND(misc-redundant-expression)
}  // namespace detail

/// @brief Generates a new Function that is the composition of @a aFunctionOuter and @a aFunctionInner, i.e.
/// \f$f(g(x))\f$
/// @param aFunctionOuter The outer function, \f$f\f$ in \f$f(g(x))\f$
/// @param aFunctionInner The inner function, \f$g\f$ in \f$f(g(x))\f$
/// @note Derivatives are implemented using the chain rule.
/// @note The definitions of @a FunctionTypeOuter and @a FunctionTypeInner must match in the sense that the outer
/// function must implement a subset of the functions implemented in the inner function. For example, if the outer
/// function implements function evaluation and the first derivative, but the inner function implements only
/// function evaluation, that will result in a compilation error. However, if the inner function implements evaluation,
/// first derivative, and first derivative with adjoint ordering, then the resulting composed function will only
/// implement evaluation and first derivative.
template <typename FunctionTypeOuter, typename FunctionTypeInner>
    requires(detail::kFunctionCompositionSizesRequirement<FunctionTypeOuter, FunctionTypeInner>)
auto compose(const FunctionTypeOuter& aFunctionOuter, const FunctionTypeInner& aFunctionInner);

namespace detail
{
template <std::size_t kIndex, typename Tuple, typename FunctionTypeOuter, typename FunctionTypeInner>
auto compose_impl(Tuple&& aTuple, FunctionTypeOuter aFunctionOuter, FunctionTypeInner aFunctionInner)
{
    using InfoAtIndex = std::tuple_element_t<kIndex, typename FunctionTypeOuter::TupleHelper::InfoTuple>;
    using Domain = typename FunctionTypeInner::Domain;
    if constexpr (InfoAtIndex::mOrder == 0)
    {
        auto tFOfG = [tOuter = std::move(aFunctionOuter), tInner = std::move(aFunctionInner)](Domain aDomain)
        { return tOuter.template evaluate<0>(tInner.template evaluate<0>(aDomain)); };
        return std::tuple_cat(std::forward<Tuple>(aTuple), std::make_tuple(std::move(tFOfG)));
    }
    else if constexpr (InfoAtIndex::mOrder == 1 && InfoAtIndex::mOrdering == MatrixOrdering::kOriginal)
    {
        auto tDfOfG = [tOuter = std::move(aFunctionOuter), tInner = std::move(aFunctionInner)](Domain aDomain)
        {
            return tOuter.template evaluate<1>(tInner.template evaluate<0>(aDomain)) *
                   tInner.template evaluate<1>(aDomain);
        };
        return std::tuple_cat(std::forward<Tuple>(aTuple), std::make_tuple(std::move(tDfOfG)));
    }
    else if constexpr (InfoAtIndex::mOrder == 1 && InfoAtIndex::mOrdering == MatrixOrdering::kAdjoint)
    {
        auto tDfOfG = [tOuter = std::move(aFunctionOuter), tInner = std::move(aFunctionInner)](Domain aDomain)
        {
            return tInner.template evaluate<1, MatrixOrdering::kAdjoint>(aDomain) *
                   tOuter.template evaluate<1, MatrixOrdering::kAdjoint>(tInner.template evaluate<0>(aDomain));
        };
        return std::tuple_cat(std::forward<Tuple>(aTuple), std::make_tuple(std::move(tDfOfG)));
    }
    else
    {
        static_assert(InfoAtIndex::mOrder < 2, "Higher-order derivatives not yet implemented in compose.");
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
[[nodiscard]] constexpr auto info_tuple_with_composition_codomains_impl(CurrentTuple&& aCurrentTuple)
{
    if constexpr (kIndex < std::tuple_size_v<InfoTuple>)
    {
        using CodomainType = std::invoke_result_t<std::tuple_element_t<kIndex, FunctionCompositionTuple>, Domain>;
        using CurrentFunctionInfo = std::tuple_element_t<kIndex, InfoTuple>;
        using ComposedFunctionInfo =
            FunctionInfo<CodomainType, CurrentFunctionInfo::mOrder, CurrentFunctionInfo::mOrdering>;
        return info_tuple_with_composition_codomains_impl<kIndex + 1, Domain, FunctionCompositionTuple, InfoTuple>(
            std::tuple_cat(std::forward<CurrentTuple>(aCurrentTuple), std::make_tuple(ComposedFunctionInfo{})));
    }
    else
    {
        return std::forward<CurrentTuple>(aCurrentTuple);
    }
}

template <typename Domain, typename FunctionCompositionTuple, typename InfoTuple>
[[nodiscard]] constexpr auto info_tuple_with_composition_codomains()
{
    static_assert(std::tuple_size_v<FunctionCompositionTuple> == std::tuple_size_v<InfoTuple>,
                  "info_tuple_with_composition_codomains must be instantiated with tuples of the same size.");

    constexpr auto tStartIndex = std::size_t{0};
    return info_tuple_with_composition_codomains_impl<tStartIndex, Domain, FunctionCompositionTuple, InfoTuple>(
        std::tuple{});
}

template <typename Domain, typename InfoTuple, typename FunctionTuple, std::size_t... kIndices>
[[nodiscard]] constexpr auto function_from_info_tuple(std::index_sequence<kIndices...> /*unused*/)
{
    using CompositionInfoTuple = decltype(info_tuple_with_composition_codomains<Domain, FunctionTuple, InfoTuple>());
    return Function<Domain, std::tuple_element_t<kIndices, CompositionInfoTuple>...>{
        std::function<typename std::tuple_element_t<kIndices, CompositionInfoTuple>::Codomain(Domain)>{}...};
}
}  // namespace detail

template <typename FunctionTypeOuter, typename FunctionTypeInner>
    requires(detail::kFunctionCompositionSizesRequirement<FunctionTypeOuter, FunctionTypeInner>)
auto compose(const FunctionTypeOuter& aFunctionOuter, const FunctionTypeInner& aFunctionInner)
{
    constexpr auto tStartingIndex = 0;
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
