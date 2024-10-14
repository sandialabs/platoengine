#ifndef PLATO_CORE_PARALLELAGGREGATE
#define PLATO_CORE_PARALLELAGGREGATE

#include <boost/mpi.hpp>

#include "plato/core/Aggregate.hpp"
#include "plato/core/Function.hpp"

namespace plato::core
{
/// @brief Implements a weighted sum of Function objects computed via MPI parallelization.
///
/// Requires that the return types represented in `Info` implement addition, as well as multiplication with a scalar.
/// Further requires that the return types can be communicated using boost mpi. Built-in types are
/// handled with native MPI functions, but standard library types and user-defined types must be
/// serialized with boost serialization.
/// @sa make_aggregate_function_with_first_derivative
template <typename DomainType, typename... Info>
class ParallelAggregate
{
   public:
    using AggregateFunction = Function<DomainType, Info...>;

    ParallelAggregate(std::vector<std::pair<AggregateFunction, double>> aFunctionsAndWeights,
                      boost::mpi::communicator aCommunicator);

    /// @brief Computes the weighted sum of functions owned by this object and
    ///  then performs a blocking all reduce operation on the result.
    template <int Order, MatrixOrdering Ordering = MatrixOrdering::kOriginal, typename Argument>
    [[nodiscard]] auto evaluate(Argument&& aArgument) const;

    /// @return The number of Function objects used on construction.
    [[nodiscard]] std::size_t size() const;

   private:
    Aggregate<DomainType, Info...> mAggregateFunction;
    boost::mpi::communicator mCommunicator{};
};

/// @brief Creates a Function object from an Aggregate.
template <typename DomainType, typename... Info>
[[nodiscard]] auto make_aggregate_function_with_first_derivative(
    const ParallelAggregate<DomainType, Info...>& aAggregate)
{
    return make_function_with_first_derivative(
        [aAggregate](DomainType aArg) { return aAggregate.template evaluate<0>(std::move(aArg)); },
        [aAggregate](DomainType aArg) { return aAggregate.template evaluate<1>(std::move(aArg)); });
}

template <typename DomainType, typename... Info>
ParallelAggregate<DomainType, Info...>::ParallelAggregate(
    std::vector<std::pair<AggregateFunction, double>> aFunctionsAndWeights, boost::mpi::communicator aCommunicator)
    : mAggregateFunction{std::move(aFunctionsAndWeights)}, mCommunicator{std::move(aCommunicator)}
{
}

template <typename DomainType, typename... Info>
template <int Order, MatrixOrdering Ordering, typename Argument>
auto ParallelAggregate<DomainType, Info...>::evaluate(Argument&& aArgument) const
{
    auto tResult = mAggregateFunction.template evaluate<Order, Ordering>(std::forward<Argument>(aArgument));
    boost::mpi::all_reduce(mCommunicator, boost::mpi::inplace(tResult), std::plus<decltype(tResult)>());
    return tResult;
}

template <typename DomainType, typename... Info>
auto ParallelAggregate<DomainType, Info...>::size() const -> std::size_t
{
    return mAggregateFunction.size();
}

}  // namespace plato::core

#endif
