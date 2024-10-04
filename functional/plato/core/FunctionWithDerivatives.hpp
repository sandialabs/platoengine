#ifndef PLATO_CORE_FUNCTIONWITHDERIVATIVES
#define PLATO_CORE_FUNCTIONWITHDERIVATIVES

#include <functional>
#include <tuple>

namespace plato::core
{

enum struct MatrixOrdering
{
    kNormal,
    kAdjoint
};

template <typename CodomainType, int kOrder, MatrixOrdering kOrdering = MatrixOrdering::kNormal>
struct FunctionInfo
{
    using Codomain = CodomainType;
    constexpr static int order = kOrder;
    constexpr static MatrixOrdering ordering = kOrdering;
};

namespace detail
{
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
};
}  // namespace detail

/// @brief A wrapper for a function with its derivatives.
///
/// Functions are labeled with their derivative order (0, 1, 2, etc.) as well as a MatrixOrdering,
/// indicating if the operation involves a matrix, whether to use the matrix or its transpose.
///
/// As an example, the following defines a scalar function with one derivative
/// @code{.cpp}
/// using ScalarFInfo = FunctionInfo<double, 0>;
/// using ScalarFirstDerivativeInfo = FunctionInfo<double, 1>;
/// using ScalarFunctionWithFirstDerivative = FunctionWithDerivatives<double, ScalarFInfo, ScalarFirstDerivativeInfo>;
///
/// const auto tFunction = ScalarFunctionWithFirstDerivative{[](const double x){ return x * x;}, [](const double x){
/// return 2.0 * x; }};
/// @endcode
template <typename Domain, typename... Info>
class FunctionWithDerivatives
{
   public:
    using TupleHelper = detail::MakeTupleHelper<Domain, Info...>;
    using FunctionTuple = typename TupleHelper::FunctionTuple;

    template <typename... Functions>
    FunctionWithDerivatives(Functions&&... aFunctions);

    template <int Order, MatrixOrdering Ordering = MatrixOrdering::kNormal, typename Argument>
    auto f(Argument&& aArgument) const;

   private:
    FunctionTuple mFunctions;
};

namespace detail
{
template <typename Domain, typename... Info>
template <int kOrder, MatrixOrdering kOrdering, std::size_t kIndex>
constexpr auto MakeTupleHelper<Domain, Info...>::get_index_impl() -> std::size_t
{
    using InfoAtIndex = std::tuple_element_t<kIndex, InfoTuple>;
    if constexpr (InfoAtIndex::order == kOrder && InfoAtIndex::ordering == kOrdering)
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
}  // namespace detail

template <typename Domain, typename... Info>
template <typename... Functions>
FunctionWithDerivatives<Domain, Info...>::FunctionWithDerivatives(Functions&&... aFunctions)
    : mFunctions{std::forward_as_tuple(aFunctions...)}
{
}

template <typename Domain, typename... Info>
template <int Order, MatrixOrdering Ordering, typename Argument>
auto FunctionWithDerivatives<Domain, Info...>::f(Argument&& aArgument) const
{
    constexpr auto tIndex = TupleHelper::template index<Order, Ordering>();
    const auto& tFunction = std::get<tIndex>(mFunctions);
    return tFunction(std::forward<Argument>(aArgument));
}

}  // namespace plato::core

#endif
