#ifndef PLATO_CORE_AGGREGATE
#define PLATO_CORE_AGGREGATE

#include <algorithm>
#include <numeric>
#include <utility>
#include <vector>

#include "plato/core/Function.hpp"

namespace plato::core
{
/// @brief Implements a weighted sum of Function objects.
///
/// Requires that all return types of each function represented in the Info types implement addition,
/// as well as multiplication with a scalar.
/// @sa make_aggregate_function_with_first_derivative
template <typename DomainType, typename... Info>
class Aggregate
{
   public:
    using AggregateFunction = Function<DomainType, Info...>;

    explicit Aggregate(std::vector<std::pair<AggregateFunction, double>> aFunctionsAndWeights);

    /// @brief Computes a weighted sum of all Function object held by this object, evaluated
    /// for the derivative @a Order and matrix ordering @a Ordering.
    template <int Order, MatrixOrdering Ordering = MatrixOrdering::kOriginal, typename Argument>
    [[nodiscard]] auto evaluate(const Argument& aArgument) const;

    /// @return The number of Function objects used on construction.
    [[nodiscard]] std::size_t size() const;

    [[nodiscard]] std::vector<double> weights() const;

   private:
    using FunctionAndWeight = std::pair<AggregateFunction, double>;
    std::vector<FunctionAndWeight> mFunctionsAndWeights;
};

/// @brief Creates a Function object from an Aggregate.
template <template <typename, typename...> typename AggregateType, typename DomainType, typename... Info>
[[nodiscard]] auto make_aggregate_function_with_first_derivative(const AggregateType<DomainType, Info...>& aAggregate)
{
    return make_function_with_first_derivative(
        [aAggregate](DomainType aArg) { return aAggregate.template evaluate<0>(std::move(aArg)); },
        [aAggregate](DomainType aArg) { return aAggregate.template evaluate<1>(std::move(aArg)); });
}

/// @brief Creates a Function object by constructing an Aggregate from vectors of Functions and weights.
template <typename DomainType, typename... Info>
[[nodiscard]] auto make_aggregate_function_with_first_derivative(
    std::vector<std::pair<Function<DomainType, Info...>, double>> aFunctionsAndWeights)
{
    return make_aggregate_function_with_first_derivative(Aggregate{std::move(aFunctionsAndWeights)});
}

template <typename DomainType, typename... Info>
Aggregate<DomainType, Info...>::Aggregate(
    std::vector<std::pair<Function<DomainType, Info...>, double>> aFunctionsAndWeights)
    : mFunctionsAndWeights(std::move(aFunctionsAndWeights))
{
}

template <typename DomainType, typename... Info>
template <int Order, MatrixOrdering Ordering, typename Argument>
auto Aggregate<DomainType, Info...>::evaluate(const Argument& aArgument) const
{
    if (mFunctionsAndWeights.empty())
    {
        using ReturnType = typename AggregateFunction::template Codomain<Order, Ordering>;
        return ReturnType{};
    }

    auto tResult = mFunctionsAndWeights.front().second *
                   mFunctionsAndWeights.front().first.template evaluate<Order, Ordering>(aArgument);

    return std::accumulate(std::next(mFunctionsAndWeights.cbegin()), mFunctionsAndWeights.cend(), std::move(tResult),
                           [&aArgument](const auto& aResult, const FunctionAndWeight& aFunctionAndWeight)
                           {
                               const auto& [tFunction, tWeight] = aFunctionAndWeight;
                               return aResult + tWeight * tFunction.template evaluate<Order, Ordering>(aArgument);
                           });
}

template <typename DomainType, typename... Info>
auto Aggregate<DomainType, Info...>::size() const -> std::size_t
{
    return mFunctionsAndWeights.size();
}

template <typename DomainType, typename... Info>
auto Aggregate<DomainType, Info...>::weights() const -> std::vector<double>
{
    std::vector<double> tWeights;
    std::transform(mFunctionsAndWeights.cbegin(), mFunctionsAndWeights.cend(), std::back_inserter(tWeights),
                   [](const FunctionAndWeight& aElement) { return aElement.second; });
    return tWeights;
}
}  // namespace plato::core

#endif
