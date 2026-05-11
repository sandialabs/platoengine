#ifndef PLATO_CORE_PARALLELFUNCTION
#define PLATO_CORE_PARALLELFUNCTION

#include <boost/mpi/collectives/broadcast.hpp>
#include <boost/mpi/communicator.hpp>

#include "plato/core/Compose.hpp"
#include "plato/core/Function.hpp"

namespace plato::core
{
/// @brief Wraps a Function so that only rank 0 returns the objective value and gradient.
///
/// The purpose of this function is to help create a Function that runs in parallel with a specific
/// communicator and adapt it to the behavior of the Aggregate classes. The Aggregate classes use
/// all_reduce to sum objectives and gradients, which would not be correct for parallelized Function
/// objects that return the same value on all ranks. This uses @a aCommunicator so that only rank 0 on @a aCommunicator
/// returns the value, and the other ranks return 0.
template <typename F>
[[nodiscard]] auto adapt_parallel_function(F aFun, const boost::mpi::communicator& aCommunicator);

/// @brief Given a value @a aValue computed on comm @a aCommunicator, returns the corresponding value computed on root.
template <typename Type>
[[nodiscard]] Type broadcast_from_root(const boost::mpi::communicator& aCommunicator, Type aValue);

namespace detail
{
constexpr static inline auto kRootRank = 0;

[[nodiscard]] inline double rank_weight(const boost::mpi::communicator& aCommunicator)
{
    return aCommunicator.rank() == kRootRank ? 1.0 : 0.0;
}
}  // namespace detail

template <typename F>
auto adapt_parallel_function(F aFun, const boost::mpi::communicator& aCommunicator)
{
    using FunctionReturn = typename F::template Codomain<0>;

    const auto tParallelAdapter = make_function_with_first_derivative(
        [tCommunicator = aCommunicator](const FunctionReturn& aArg)
        { return detail::rank_weight(tCommunicator) * aArg; },
        [tCommunicator = aCommunicator](const FunctionReturn&) { return detail::rank_weight(tCommunicator); });

    return core::compose(tParallelAdapter, std::move(aFun));
}

template <typename Type>
Type broadcast_from_root(const boost::mpi::communicator& aCommunicator, Type aValue)
{
    boost::mpi::broadcast(aCommunicator, aValue, detail::kRootRank);
    return aValue;
}

}  // namespace plato::core

#endif
