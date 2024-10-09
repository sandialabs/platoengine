#ifndef PLATO_CORE_FUNCTIONWITHDERIVATIVES
#define PLATO_CORE_FUNCTIONWITHDERIVATIVES

#include <functional>
#include <tuple>

#include "plato/core/FunctionHelpers.hpp"

namespace plato::core
{
/// Convenience variables for working with Function.
namespace evaluate
{
static constexpr auto kFunction = int{0};
static constexpr auto kFirstDerivative = int{1};
static constexpr auto kSecondDerivative = int{2};
}  // namespace evaluate

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
template <typename DomainType, typename... Info>
class FunctionWithDerivatives
{
   public:
    using Domain = DomainType;
    using TupleHelper = detail::MakeTupleHelper<DomainType, Info...>;
    using FunctionTuple = typename TupleHelper::FunctionTuple;

    template <typename... Functions>
    FunctionWithDerivatives(Functions&&... aFunctions);

    template <int Order, MatrixOrdering Ordering = MatrixOrdering::kOriginal, typename Argument>
    auto evaluate(Argument&& aArgument) const;

    template <int Order, MatrixOrdering Ordering>
    constexpr static auto isImplemented() -> bool;

   private:
    FunctionTuple mFunctions;
};

template <typename Domain, typename... Info>
template <typename... Functions>
FunctionWithDerivatives<Domain, Info...>::FunctionWithDerivatives(Functions&&... aFunctions)
    : mFunctions{std::forward_as_tuple(aFunctions...)}
{
}

template <typename Domain, typename... Info>
template <int Order, MatrixOrdering Ordering, typename Argument>
auto FunctionWithDerivatives<Domain, Info...>::evaluate(Argument&& aArgument) const
{
    static_assert(isImplemented<Order, Ordering>(),
                  "Requested function derivative order or matrix ordering not implemented.");

    constexpr auto tIndex = TupleHelper::template index<Order, Ordering>();
    const auto& tFunction = std::get<tIndex>(mFunctions);
    return tFunction(std::forward<Argument>(aArgument));
}

template <typename Domain, typename... Info>
template <int Order, MatrixOrdering Ordering>
constexpr auto FunctionWithDerivatives<Domain, Info...>::isImplemented() -> bool
{
    return TupleHelper::template is_implemented<Order, Ordering>();
}

}  // namespace plato::core

#endif
