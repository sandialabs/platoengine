#ifndef PLATO_TEST_UTILITIES_PARALLELTESTWRAPPER
#define PLATO_TEST_UTILITIES_PARALLELTESTWRAPPER

#include <boost/mpi.hpp>
#include <boost/mpi/communicator.hpp>

#include "plato/core/Function.hpp"

namespace plato::test_utilities
{
/// @brief The purpose of this class is to facilitate parallel testing by wrapping an
///  existing serial function and adding some useless parallelism.
template <typename R, typename Arg>
class ParallelTestFunctionWrapper
{
   public:
    template <typename F>
    ParallelTestFunctionWrapper(F aFunction);

    R operator()(const Arg& aArg, const boost::mpi::communicator& aComm) const;

   private:
    std::function<R(Arg)> mFunction;
};

namespace detail
{
[[nodiscard]] inline double rank_weight(const boost::mpi::communicator& aComm) { return aComm.rank() == 0 ? 1.0 : 0.0; }
}  // namespace detail

template <typename R, typename Arg>
template <typename F>
ParallelTestFunctionWrapper<R, Arg>::ParallelTestFunctionWrapper(F aFunction) : mFunction(std::move(aFunction))
{
}

template <typename R, typename Arg>
R ParallelTestFunctionWrapper<R, Arg>::operator()(const Arg& aArg, const boost::mpi::communicator& aComm) const
{
    auto tResult = detail::rank_weight(aComm) * mFunction(aArg);
    boost::mpi::all_reduce(aComm, boost::mpi::inplace(tResult), std::plus<R>());
    return tResult;
}

}  // namespace plato::test_utilities

#endif