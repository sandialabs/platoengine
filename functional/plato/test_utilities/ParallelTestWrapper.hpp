#ifndef PLATO_TEST_UTILITIES_PARALLELTESTWRAPPER
#define PLATO_TEST_UTILITIES_PARALLELTESTWRAPPER

#include <boost/mpi.hpp>
#include <boost/mpi/communicator.hpp>
#include <functional>

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

    auto operator()(const Arg& aArg, const boost::mpi::communicator& aCommunicator) const -> R;

   private:
    std::function<R(Arg)> mFunction;
};

namespace detail
{
[[nodiscard]] inline auto rank_weight(const boost::mpi::communicator& aCommunicator) -> double
{
    return aCommunicator.rank() == 0 ? 1.0 : 0.0;
}
}  // namespace detail

template <typename R, typename Arg>
template <typename F>
ParallelTestFunctionWrapper<R, Arg>::ParallelTestFunctionWrapper(F aFunction) : mFunction(std::move(aFunction))
{
}

template <typename R, typename Arg>
auto ParallelTestFunctionWrapper<R, Arg>::operator()(const Arg& aArg,
                                                     const boost::mpi::communicator& aCommunicator) const -> R
{
    auto tResult = detail::rank_weight(aCommunicator) * mFunction(aArg);
    boost::mpi::all_reduce(aCommunicator, boost::mpi::inplace(tResult), std::plus<R>());
    return tResult;
}

}  // namespace plato::test_utilities

#endif
