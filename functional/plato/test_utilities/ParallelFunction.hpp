#ifndef PLATO_TEST_UTILITIES_PARALLELFUNCTION
#define PLATO_TEST_UTILITIES_PARALLELFUNCTION

#include <boost/mpi/communicator.hpp>

#include "plato/core/Function.hpp"
#include "plato/utilities/FunctionArgType.hpp"

namespace plato::test_utilities
{
/// @brief A helper to create a parallelized test Function by binding @a aComm to calls to two functions.
///
/// Specifically, this creates a Function from two functions @a aFun and @a aDFun, which take
/// a `boost::mpi::communicator` as a second argument and binds @a aComm to that argument on
/// calls to `f` and `df`.
template <typename F, typename DF>
auto make_parallel_function(F aFun, DF aDFun, const boost::mpi::communicator& aComm);

template <typename F, typename DF>
auto make_parallel_function(F aFun, DF aDFun, const boost::mpi::communicator& aComm)
{
    static_assert(std::is_convertible_v<typename utilities::FunctionArgType<F>::template arg<1U>,
                                        const boost::mpi::communicator&>);
    static_assert(std::is_convertible_v<typename utilities::FunctionArgType<DF>::template arg<1U>,
                                        const boost::mpi::communicator&>);

    using ArgF = typename utilities::FunctionArgType<F>::template arg<0U>;
    using ArgDF = typename utilities::FunctionArgType<DF>::template arg<0U>;

    return core::make_function_with_first_derivative(
        [tComm = aComm, tFun = std::move(aFun)](ArgF aArg) { return tFun(aArg, tComm); },
        [tComm = aComm, tDFun = std::move(aDFun)](ArgDF aArg) { return tDFun(aArg, tComm); });
}
}  // namespace plato::test_utilities

#endif
