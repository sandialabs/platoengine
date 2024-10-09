#ifndef PLATO_CORE_FUNCTION
#define PLATO_CORE_FUNCTION

#include <functional>
#include <tuple>
#include <type_traits>

#include "plato/core/FunctionHelpers.hpp"

namespace plato::core
{
/// Convenience variables for working with Function.
namespace evaluation
{
static constexpr auto kFunction = int{0};
static constexpr auto kFirstDerivative = int{1};
static constexpr auto kSecondDerivative = int{2};
}  // namespace evaluation

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

/// @brief A helper class for evaluating a function and its derivative.
/// @tparam R The return type of the function.
/// @tparam dR The return type of the derivative of the function.
/// @tparam Arg The type of the function argument.
template <typename R, typename dR, typename Arg>
class Function
{
   public:
    using FunctionReturn = R;

   public:
    /// Construction from two function objects, @a fun evaluates the function and @a dfun its derivative.
    template <typename F, typename DF>
    Function(F fun, DF dfun);

    /// Evaluate the function at @a arg.
    [[nodiscard]] R f(const Arg& arg) const;

    /// Evaluate the function derivative at @a arg.
    [[nodiscard]] dR df(const Arg& arg) const;

   private:
    std::function<R(Arg)> mF;
    std::function<dR(Arg)> mDF;
};

/// Helper function for constructing a Function. The types of the arguments and Function can be deduced.
template <typename F, typename DF>
auto make_function(F aF, DF aDf);

namespace detail
{
/// Helper for deducing the argument type of a callable object @a F
/// @a F must have an `operator()` defined.
template <typename F>
struct ArgType
{
};

template <typename F, typename R, typename Arg>
struct ArgType<R (F::*)(Arg)>
{
    using type = Arg;
};

template <typename F, typename R, typename Arg>
struct ArgType<R (F::*)(Arg) const>
{
    using type = Arg;
};
}  // namespace detail

/// @brief A helper function template for constructing a Function object.
///
/// The main purpose of this function is for constructing Function objects
/// using template argument deduction. Prefer usage of this over Function's ctor.
template <typename F, typename G>
auto make_function(F aF, G aG)
{
    using ArgF = typename detail::ArgType<decltype(&F::operator())>::type;
    using ArgG = typename detail::ArgType<decltype(&G::operator())>::type;
    static_assert(std::is_convertible_v<ArgF, ArgG>,
                  "The arguments of functions with type F and G must be implicitly convertible.");

    using R = std::invoke_result_t<F, ArgF>;
    using dR = std::invoke_result_t<G, ArgG>;
    return core::Function<R, dR, ArgF>{std::move(aF), std::move(aG)};
}

template <typename R, typename dR, typename Arg>
template <typename F, typename DF>
Function<R, dR, Arg>::Function(F fun, DF dfun) : mF(std::move(fun)), mDF(std::move(dfun))
{
}

template <typename R, typename dR, typename Arg>
R Function<R, dR, Arg>::f(const Arg& arg) const
{
    return mF(arg);
}

template <typename R, typename dR, typename Arg>
dR Function<R, dR, Arg>::df(const Arg& arg) const
{
    return mDF(arg);
}

}  // namespace plato::core

#endif
