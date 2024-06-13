#ifndef PLATO_TEST_UTILITIES_PARALLELFUNCTION
#define PLATO_TEST_UTILITIES_PARALLELFUNCTION

#include <boost/mpi/communicator.hpp>

#include "plato/core/Function.hpp"

namespace plato::test_utilities
{
/// @brief A helper to create a parallelized test Function by binding @a aComm to calls to two functions.
///
/// Specifically, this creates a Function from two functions @a aFun and @a aDFun, which take
/// a `boost::mpi::communicator` as a second argument and binds @a aComm to that argument on
/// calls to `f` and `df`.
template <typename F, typename DF>
auto make_parallel_function(F aFun, DF aDFun, const boost::mpi::communicator& aComm);

namespace detail
{
template <typename F>
struct ParallelArgType
{
};

template <typename F, typename R, typename Arg>
struct ParallelArgType<R (F::*)(Arg, const boost::mpi::communicator&)>
{
    using type = Arg;
};

template <typename F, typename R, typename Arg>
struct ParallelArgType<R (F::*)(Arg, const boost::mpi::communicator&) const>
{
    using type = Arg;
};
}  // namespace detail

template <typename F, typename DF>
auto make_parallel_function(F aFun, DF aDFun, const boost::mpi::communicator& aComm)
{
    using ArgF = typename detail::ParallelArgType<decltype(&F::operator())>::type;
    using ArgDF = typename detail::ParallelArgType<decltype(&DF::operator())>::type;
    return core::make_function([tComm = aComm, tFun = std::move(aFun)](ArgF aArg) { return tFun(aArg, tComm); },
                               [tComm = aComm, tDFun = std::move(aDFun)](ArgDF aArg) { return tDFun(aArg, tComm); });
}
}  // namespace plato::test_utilities

#endif
