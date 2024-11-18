#ifndef PLATO_CORE_FUNCTION
#define PLATO_CORE_FUNCTION

#include <functional>
#include <tuple>
#include <type_traits>

#include "plato/core/FunctionHelpers.hpp"
#include "plato/utilities/FunctionArgType.hpp"

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

    /// @brief Converts (with copy semantics) to a Function with possibly different functions implemented.
    /// @tparam LHSDomainType The domain type of the function to convert to, must be the same as DomainType of this
    /// object.
    /// @tparam LHSInfo The function info for the function to convert to. This must be a subset of the function info for
    /// this object.
    template <typename CompatibleFunction,
              typename = std::enable_if<detail::is_copyable<typename CompatibleFunction::Domain,
                                                            typename CompatibleFunction::TupleHelper::InfoTuple,
                                                            DomainType,
                                                            std::tuple<Info...>>()>>
    [[nodiscard]] auto compatibleFunction() const& -> CompatibleFunction;

    /// @brief Converts (with move semantics) to a Function with possibly different functions implemented.
    /// @tparam LHSDomainType The domain type of the function to convert to, must be the same as DomainType of this
    /// object.
    /// @tparam LHSInfo The function info for the function to convert to. This must be a subset of the function info for
    /// this object.
    template <typename CompatibleFunction,
              typename = std::enable_if<detail::is_copyable<typename CompatibleFunction::Domain,
                                                            typename CompatibleFunction::TupleHelper::InfoTuple,
                                                            DomainType,
                                                            std::tuple<Info...>>()>>
    [[nodiscard]] auto compatibleFunction() && -> CompatibleFunction;

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

namespace detail
{
template <std::size_t kIndex,
          typename ToFunction,
          typename FromFunction,
          typename ToFunctionsTuple,
          typename AllFunctionsTuple>
void assign_function(ToFunctionsTuple& aToFunctions, AllFunctionsTuple&& aAllFunctionsTuple)
{
    using ToInfoTuple = typename ToFunction::TupleHelper::InfoTuple;
    constexpr auto tOrder = std::tuple_element_t<kIndex, ToInfoTuple>::order;
    constexpr auto tOrdering = std::tuple_element_t<kIndex, ToInfoTuple>::ordering;
    constexpr auto tToFunctionIndex = ToFunction::TupleHelper::template index<tOrder, tOrdering>();
    constexpr auto tFromFunctionIndex = FromFunction::TupleHelper::template index<tOrder, tOrdering>();
    std::get<tToFunctionIndex>(aToFunctions) =
        std::get<tFromFunctionIndex>(std::forward<AllFunctionsTuple>(aAllFunctionsTuple));
}

template <typename ToFunction, typename FromFunction, typename AllFunctionsTuple, std::size_t... kCompatibleInfoIndices>
auto compatible_functions(AllFunctionsTuple&& aAllFunctionsTuple, std::index_sequence<kCompatibleInfoIndices...>)
{
    using ToFunctionTuple = typename ToFunction::FunctionTuple;
    auto tToFunctions = ToFunctionTuple{};
    (assign_function<kCompatibleInfoIndices, ToFunction, FromFunction>(
         tToFunctions, std::forward<AllFunctionsTuple>(aAllFunctionsTuple)),
     ...);
    return tToFunctions;
}
}  // namespace detail

template <typename Domain, typename... Info>
template <typename CompatibleFunction, typename>
auto Function<Domain, Info...>::compatibleFunction() const& -> CompatibleFunction
{
    using CompatibleInfoTuple = typename CompatibleFunction::TupleHelper::InfoTuple;
    return std::make_from_tuple<CompatibleFunction>(detail::compatible_functions<CompatibleFunction, Function>(
        mFunctions, std::make_index_sequence<std::tuple_size_v<CompatibleInfoTuple>>()));
}

template <typename Domain, typename... Info>
template <typename CompatibleFunction, typename>
auto Function<Domain, Info...>::compatibleFunction() && -> CompatibleFunction
{
    using CompatibleInfoTuple = typename CompatibleFunction::TupleHelper::InfoTuple;
    return std::make_from_tuple<CompatibleFunction>(detail::compatible_functions<CompatibleFunction, Function>(
        std::move(mFunctions), std::make_index_sequence<std::tuple_size_v<CompatibleInfoTuple>>()));
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

template <typename F, typename G>
auto make_function_with_first_derivative(F aF, G aG)
{
    using ArgF = typename utilities::FunctionArgType<F>::template arg<0U>;
    using ArgG = typename utilities::FunctionArgType<G>::template arg<0U>;
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
