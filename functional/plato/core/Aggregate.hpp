#ifndef PLATO_CORE_AGGREGATE
#define PLATO_CORE_AGGREGATE

#include <numeric>
#include <utility>
#include <vector>

#include "plato/core/Function.hpp"

namespace plato::core
{
/// @brief Implements a weighted sum of Function objects.
///
/// Requires that the types @a R and @a dR implement addition,
/// as well as multiplication with a scalar.
/// @sa make_aggregate_function
template <typename R, typename dR, typename Arg>
class Aggregate
{
   public:
    using AggregateFunction = Function<R, dR, Arg>;

    explicit Aggregate(std::vector<std::pair<AggregateFunction, double>> aFunctionsAndWeights);

    [[nodiscard]] R f(const Arg& aArg) const;
    [[nodiscard]] dR df(const Arg& aArg) const;

    /// @return The number of Function objects used on construction.
    [[nodiscard]] std::size_t size() const;

    [[nodiscard]] std::vector<double> weights() const;

   private:
    using FunctionAndWeight = std::pair<AggregateFunction, double>;
    std::vector<FunctionAndWeight> mFunctionsAndWeights;
};

/// @brief Creates a Function object from an Aggregate.
template <typename R, typename dR, typename Arg>
[[nodiscard]] auto make_aggregate_function(const Aggregate<R, dR, Arg>& aAggregate)
{
    return make_function([aAggregate](const Arg& aArg) { return aAggregate.f(aArg); },
                         [aAggregate](const Arg& aArg) { return aAggregate.df(aArg); });
}

/// @brief Creates a Function object by constructing an Aggregate from vectors of Functions and weights.
template <typename R, typename dR, typename Arg>
[[nodiscard]] auto make_aggregate_function(std::vector<std::pair<Function<R, dR, Arg>, double>> aFunctionsAndWeights)
{
    return make_aggregate_function(Aggregate{std::move(aFunctionsAndWeights)});
}

template <typename R, typename dR, typename Arg>
Aggregate<R, dR, Arg>::Aggregate(std::vector<std::pair<Function<R, dR, Arg>, double>> aFunctionsAndWeights)
    : mFunctionsAndWeights(std::move(aFunctionsAndWeights))
{
    // Use requires clause in c++20
    // Note that these expressions aren't quite right because missing operator* or operator+ makes this fail to compile,
    // not trip the static_assert. However, this should still help understand compiler errors.
    static_assert(std::is_convertible_v<decltype(1.0 * std::declval<R>()), R>,
                  "Aggregate template parameter R must implement scalar multiplication.");
    static_assert(std::is_convertible_v<decltype(std::declval<R>() + std::declval<R>()), R>,
                  "Aggregate template parameter R must implement addition.");
    static_assert(std::is_convertible_v<decltype(1.0 * std::declval<dR>()), dR>,
                  "Aggregate template parameter dR must implement scalar multiplication.");
    static_assert(std::is_convertible_v<decltype(std::declval<dR>() + std::declval<dR>()), dR>,
                  "Aggregate template parameter dR must implement addition.");
}

template <typename R, typename dR, typename Arg>
R Aggregate<R, dR, Arg>::f(const Arg& aArg) const
{
    return std::accumulate(mFunctionsAndWeights.cbegin(), mFunctionsAndWeights.cend(), R{},
                           [&aArg](const R& aResult, const FunctionAndWeight& aFunctionAndWeight)
                           {
                               const auto& [tFunction, tWeight] = aFunctionAndWeight;
                               return aResult + tWeight * tFunction.f(aArg);
                           });
}

template <typename R, typename dR, typename Arg>
dR Aggregate<R, dR, Arg>::df(const Arg& aArg) const
{
    if (mFunctionsAndWeights.empty())
    {
        return dR{};
    }

    dR tResult = mFunctionsAndWeights.front().second * mFunctionsAndWeights.front().first.df(aArg);
    return std::accumulate(std::next(mFunctionsAndWeights.cbegin()), mFunctionsAndWeights.cend(), std::move(tResult),
                           [&aArg](const dR& aResult, const FunctionAndWeight& aFunctionAndWeight)
                           {
                               const auto& [tFunction, tWeight] = aFunctionAndWeight;
                               return aResult + tWeight * tFunction.df(aArg);
                           });
}

template <typename R, typename dR, typename Arg>
std::size_t Aggregate<R, dR, Arg>::size() const
{
    return mFunctionsAndWeights.size();
}

template <typename R, typename dR, typename Arg>
std::vector<double> Aggregate<R, dR, Arg>::weights() const
{
    std::vector<double> tWeights;
    std::transform(mFunctionsAndWeights.cbegin(), mFunctionsAndWeights.cend(), std::back_inserter(tWeights),
                   [](const FunctionAndWeight& aElement) { return aElement.second; });
    return tWeights;
}
}  // namespace plato::core

#endif
