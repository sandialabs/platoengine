#ifndef PLATO_CORE_PARALLELAGGREGATE
#define PLATO_CORE_PARALLELAGGREGATE

#include <boost/mpi.hpp>

#include "plato/core/Aggregate.hpp"
#include "plato/core/Function.hpp"

namespace plato::core
{
/// @brief Implements a weighted sum of Function objects computed via MPI parallelization.
///
/// Requires that the types @a R and @a dR implement addition, as well as multiplication with a scalar.
/// Further requires that types @a R and @a dR can be communicated using boost mpi. Built-in types are
/// handled with native MPI functions, but standard library types and user-defined types must be
/// serialized with boost serialization.
/// @sa make_aggregate_function
template <typename R, typename dR, typename Arg>
class ParallelAggregate
{
   public:
    using AggregateFunction = Function<R, dR, Arg>;

    ParallelAggregate(std::vector<std::pair<AggregateFunction, double>> aFunctionsAndWeights,
                      boost::mpi::communicator aCommunicator);

    /// @brief Computes the weighted sum of functions owned by this object and
    ///  then performs a blocking all reduce operation on the result.
    [[nodiscard]] R f(const Arg& aArg) const;
    /// @brief Computes the weighted sum of the derivatives of the functions owned by
    ///  this object and then performs a blocking all reduce operation on the result.
    [[nodiscard]] dR df(const Arg& aArg) const;

    /// @return The number of Function objects used on construction.
    [[nodiscard]] std::size_t size() const;

   private:
    Aggregate<R, dR, Arg> mAggregateFunction;
    boost::mpi::communicator mCommunicator{};
};

/// @brief Creates a Function object from an Aggregate.
template <typename R, typename dR, typename Arg>
[[nodiscard]] auto make_aggregate_function(const ParallelAggregate<R, dR, Arg>& aAggregate)
{
    return make_function([aAggregate](const Arg& aArg) { return aAggregate.f(aArg); },
                         [aAggregate](const Arg& aArg) { return aAggregate.df(aArg); });
}

template <typename R, typename dR, typename Arg>
ParallelAggregate<R, dR, Arg>::ParallelAggregate(std::vector<std::pair<AggregateFunction, double>> aFunctionsAndWeights,
                                                 boost::mpi::communicator aCommunicator)
    : mAggregateFunction{std::move(aFunctionsAndWeights)}, mCommunicator{std::move(aCommunicator)}
{
}

template <typename R, typename dR, typename Arg>
R ParallelAggregate<R, dR, Arg>::f(const Arg& aArg) const
{
    R tResult = mAggregateFunction.f(aArg);
    boost::mpi::all_reduce(mCommunicator, boost::mpi::inplace(tResult), std::plus<R>());
    return tResult;
}

template <typename R, typename dR, typename Arg>
dR ParallelAggregate<R, dR, Arg>::df(const Arg& aArg) const
{
    dR tResult = mAggregateFunction.df(aArg);
    boost::mpi::all_reduce(mCommunicator, boost::mpi::inplace(tResult), std::plus<dR>());
    return tResult;
}

template <typename R, typename dR, typename Arg>
std::size_t ParallelAggregate<R, dR, Arg>::size() const
{
    return mAggregateFunction.size();
}

}  // namespace plato::core

#endif
