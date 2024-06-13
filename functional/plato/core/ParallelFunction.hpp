#ifndef PLATO_CORE_PARALLELFUNCTION
#define PLATO_CORE_PARALLELFUNCTION

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

namespace detail
{
[[nodiscard]] inline double rank_weight(const boost::mpi::communicator& aComm) { return aComm.rank() == 0 ? 1.0 : 0.0; }
}  // namespace detail

template <typename F>
auto adapt_parallel_function(F aFun, const boost::mpi::communicator& aComm)
{
    const auto tParallelAdapter = make_function(
        [tComm = aComm](const typename F::FunctionReturn& aArg) { return detail::rank_weight(tComm) * aArg; },
        [tComm = aComm](const typename F::FunctionReturn&) { return detail::rank_weight(tComm); });

    return core::compose(tParallelAdapter, std::move(aFun));
}

}  // namespace plato::core

#endif
