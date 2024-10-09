#ifndef PLATO_CORE_COMPOSE
#define PLATO_CORE_COMPOSE

#include <type_traits>
#include <utility>

#include "plato/core/Function.hpp"
#include "plato/core/FunctionWithDerivatives.hpp"

namespace plato::core
{
/// @brief Generates a new Function that is the composition of @a f and @a g, i.e. \f$f(g(x))\f$
template <typename fR, typename dFR, typename fArg, typename gR, typename dGR, typename gArg>
[[nodiscard]] auto compose(Function<fR, dFR, fArg> aF, Function<gR, dGR, gArg> aG)
{
    static_assert(std::is_convertible_v<gR, fArg>,
                  "The return type of aG must be convertible to the argument type of aF.");

    auto tFOfG = [aF, aG](const gArg& aX) { return aF.f(aG.f(aX)); };

    auto tDfOfG = [tF = std::move(aF), tG = std::move(aG)](const gArg& tX) { return tF.df(tG.f(tX)) * tG.df(tX); };
    using dfOfGR = std::invoke_result_t<decltype(tDfOfG), gArg>;
    return core::Function<fR, dfOfGR, gArg>{std::move(tFOfG), std::move(tDfOfG)};
}

template <typename FunctionType>
auto compose_new(const FunctionType& aFunction1, const FunctionType& aFunction2);

namespace detail
{
template <std::size_t kIndex, typename Tuple, typename FunctionType>
auto compose_impl(Tuple&& aTuple, FunctionType aFunction1, FunctionType aFunction2)
{
    using InfoAtIndex = std::tuple_element_t<kIndex, typename FunctionType::TupleHelper::InfoTuple>;
    using Domain = typename FunctionType::Domain;
    if constexpr (InfoAtIndex::order == 0)
    {
        auto tFOfG = [tF = std::move(aFunction1), tG = std::move(aFunction2)](Domain aX)
        { return tF.template evaluate<0>(tG.template evaluate<0>(aX)); };
        return std::tuple_cat(std::forward<Tuple>(aTuple), std::make_tuple(std::move(tFOfG)));
    }
    else if constexpr (InfoAtIndex::order == 1 && InfoAtIndex::ordering == MatrixOrdering::kOriginal)
    {
        auto tDfOfG = [tF = std::move(aFunction1), tG = std::move(aFunction2)](Domain aX)
        { return tF.template evaluate<1>(tG.template evaluate<0>(aX)) * tG.template evaluate<1>(aX); };
        return std::tuple_cat(std::forward<Tuple>(aTuple), std::make_tuple(std::move(tDfOfG)));
    }
    else if constexpr (InfoAtIndex::order == 1 && InfoAtIndex::ordering == MatrixOrdering::kAdjoint)
    {
        auto tDfOfG = [tF = std::move(aFunction1), tG = std::move(aFunction2)](Domain aX)
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

template <std::size_t kIndex, typename Tuple, typename FunctionType>
auto compose_sequence(Tuple&& aTuple, const FunctionType& aFunction1, const FunctionType& aFunction2)
{
    if constexpr (kIndex < std::tuple_size_v<typename FunctionType::TupleHelper::InfoTuple>)
    {
        return compose_sequence<kIndex + 1>(compose_impl<kIndex>(std::forward<Tuple>(aTuple), aFunction1, aFunction2),
                                            aFunction1, aFunction2);
    }
    else
    {
        return std::forward<Tuple>(aTuple);
    }
}
}  // namespace detail
template <typename FunctionType>
auto compose_new(const FunctionType& aFunction1, const FunctionType& aFunction2)
{
    constexpr auto tStartingIndex = int{0};
    return std::make_from_tuple<FunctionType>(
        detail::compose_sequence<tStartingIndex>(std::make_tuple(), aFunction1, aFunction2));
}

}  // namespace plato::core

#endif
