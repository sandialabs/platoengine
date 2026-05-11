#ifndef PLATO_UTILTIES_MPIUTILITIES
#define PLATO_UTILTIES_MPIUTILITIES

#include <boost/mpi/communicator.hpp>
#include <utility>

namespace plato::utilities
{

///@brief Utility function that takes a communicator @a aCommunicator and only performs the function @a aFunction with
/// arguments
///@a aArgs on the root rank.
template <typename F, typename... Args>
void execute_on_root(const boost::mpi::communicator& aCommunicator, const F& aFunction, Args&&... aArgs)
{
    aCommunicator.barrier();
    if (aCommunicator.rank() == 0)
    {
        aFunction(std::forward<Args>(aArgs)...);
    }
    aCommunicator.barrier();
}

}  // namespace plato::utilities

#endif
