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

/// @brief Helper function template for constructing a Function object.
///
/// The main purpose of this function is for constructing Function objects
/// using template argument deduction. Prefer usage of this over Function's ctor.
template <typename F, typename G>
auto make_function_with_first_derivative(F aF, G aG);

/// @brief A wrapper for a function with its derivatives.
///
/// Functions are labeled with their derivative order (0, 1, 2, etc.) as well as a MatrixOrdering,
/// indicating if the operation involves a matrix, whether to use the matrix or its transpose.
///
/// As an example, the following defines a scalar function with one derivative
/// @code{.cpp}
/// using ScalarFInfo = FunctionInfo<double, 0>;
/// using ScalarFirstDerivativeInfo = FunctionInfo<double, 1>;
/// using ScalarFunctionWithFirstDerivative = Function<double, ScalarFInfo, ScalarFirstDerivativeInfo>;
///
/// const auto tFunction = ScalarFunctionWithFirstDerivative{[](const double x){ return x * x;}, [](const double x){
/// return 2.0 * x; }};
/// @endcode
template <typename DomainType, typename... Info>
class Function
{
   public:
    using Domain = DomainType;
    using TupleHelper = detail::MakeTupleHelper<DomainType, Info...>;
    using FunctionTuple = typename TupleHelper::FunctionTuple;

    template <typename... Functions, typename = std::enable_if_t<!detail::is_only_member<Function, Functions...>()>>
    Function(Functions&&... aFunctions);

    /// @brief Evaluates a function with derivative order @a Order with matrix ordering @a Ordering.
    template <int Order, MatrixOrdering Ordering = MatrixOrdering::kOriginal, typename Argument>
    [[nodiscard]] auto evaluate(Argument&& aArgument) const;

    /// @brief Returns true if this instantiation of Function implements the derivative order @a Order and
    ///  with matrix ordering @a Ordering.
    template <int Order, MatrixOrdering Ordering>
    [[nodiscard]] constexpr static auto isImplemented() -> bool;

    /// @brief The return type of the function associated with derivative order @a Order with matrix ordering @a
    /// Ordering.
    template <int Order, MatrixOrdering Ordering = MatrixOrdering::kOriginal>
    using Codomain = typename std::tuple_element_t<TupleHelper::template index<Order, Ordering>(),
                                                   typename TupleHelper::InfoTuple>::Codomain;

   private:
    FunctionTuple mFunctions;
};

template <typename Domain, typename... Info>
template <typename... Functions, typename>
Function<Domain, Info...>::Function(Functions&&... aFunctions) : mFunctions{std::forward_as_tuple(aFunctions...)}
{
    static_assert(sizeof...(Functions) == std::tuple_size_v<FunctionTuple>,
                  "Mismatch in number of functions passed to Function constructor.");
}

template <typename Domain, typename... Info>
template <int Order, MatrixOrdering Ordering, typename Argument>
auto Function<Domain, Info...>::evaluate(Argument&& aArgument) const
{
    static_assert(isImplemented<Order, Ordering>(),
                  "Requested function derivative order or matrix ordering not implemented.");

    constexpr auto tIndex = TupleHelper::template index<Order, Ordering>();
    const auto& tFunction = std::get<tIndex>(mFunctions);
    return tFunction(std::forward<Argument>(aArgument));
}

template <typename Domain, typename... Info>
template <int Order, MatrixOrdering Ordering>
constexpr auto Function<Domain, Info...>::isImplemented() -> bool
{
    return TupleHelper::template is_implemented<Order, Ordering>();
}

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

template <typename F, typename G>
auto make_function_with_first_derivative(F aF, G aG)
{
    using ArgF = typename detail::ArgType<decltype(&F::operator())>::type;
    using ArgG = typename detail::ArgType<decltype(&G::operator())>::type;
    static_assert(std::is_convertible_v<ArgF, ArgG>,
                  "The arguments of functions with type F and G must be implicitly convertible.");

    using R = std::invoke_result_t<F, ArgF>;
    using dR = std::invoke_result_t<G, ArgG>;

    using FunctionEvaluationInfo = FunctionInfo<R, evaluation::kFunction>;
    using FunctionFirstDerivativeInfo = FunctionInfo<dR, evaluation::kFirstDerivative>;
    using FunctionType = Function<ArgF, FunctionEvaluationInfo, FunctionFirstDerivativeInfo>;

    return FunctionType{std::move(aF), std::move(aG)};
}

}  // namespace plato::core

#endif
