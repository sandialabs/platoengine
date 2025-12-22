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
/// objects that return the same value on all ranks. This uses @a aComm so that only rank 0 on @a aComm
/// returns the value, and the other ranks return 0.
template <typename F>
[[nodiscard]] auto adapt_parallel_function(F aFun, const boost::mpi::communicator& aComm);

/// @brief Given a value @a aValue computed on comm @a aComm, returns the corresponding value computed on root.
template <typename Type>
[[nodiscard]] Type broadcast_from_root(const boost::mpi::communicator& aComm, Type aValue);

namespace detail
{
constexpr static inline auto kRootRank = 0;

[[nodiscard]] inline double rank_weight(const boost::mpi::communicator& aComm)
{
    return aComm.rank() == kRootRank ? 1.0 : 0.0;
}
}  // namespace detail

template <typename F>
auto adapt_parallel_function(F aFun, const boost::mpi::communicator& aComm)
{
    using FunctionReturn = typename F::template Codomain<0>;

    const auto tParallelAdapter = make_function_with_first_derivative(
        [tComm = aComm](const FunctionReturn& aArg) { return detail::rank_weight(tComm) * aArg; },
        [tComm = aComm](const FunctionReturn&) { return detail::rank_weight(tComm); });

    return core::compose(tParallelAdapter, std::move(aFun));
}

template <typename Type>
Type broadcast_from_root(const boost::mpi::communicator& aComm, Type aValue)
{
    boost::mpi::broadcast(aComm, aValue, detail::kRootRank);
    return aValue;
}

}  // namespace plato::core

#endif
