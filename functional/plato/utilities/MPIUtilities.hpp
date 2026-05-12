#ifndef PLATO_UTILTIES_MPIUTILITIES
#define PLATO_UTILTIES_MPIUTILITIES

#include <boost/mpi/communicator.hpp>
#include <utility>

#include "plato/utilities/RankSplitVector.hpp"

namespace plato::utilities
{

///@brief Utility function that takes a communicator @a aCommunicator and only performs the function @a aFunction with
/// arguments
///@a aArgs on the root rank.
template <typename F, typename... Args>
void execute_on_root(const boost::mpi::communicator& aCommunicator, const F& aFunction, Args&&... aArgs);

/// @brief Given a parent communicator @a aParentCommunicator and a number of processors @a aNumberOfProcessors, splits
/// the parent communicator into two groups, the first with a number of ranks equal to @a aNumberOfProcessors, and the
/// second comprising the remaining ranks.
/// @pre @a aNumberOfProcessors must be less than or equal to the size of @a aParentCommunicator.
[[nodiscard]] auto group_split_communicator(const boost::mpi::communicator& aParentCommunicator,
                                            std::size_t aNumberOfProcessors)
    -> std::pair<boost::mpi::communicator, ColorNamedType>;

/// @brief Computes a function using a subset of processors
template <typename F, typename... Args>
auto compute_on_subset(const boost::mpi::communicator& aCommunicator, const F& aFunction, Args&&... aArgs);

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
/*
template <typename F, typename... Args>
auto compute_on_subset(const boost::mpi::communicator& aCommunicator, const F& aFunction, Args&&... aArgs)
{
    using ReturnType = std::invoke_result_t<F, Args...>;
    auto tResult = ReturnType{};
    if (tGroupColor.mValue == 0)
    {
        // tResult = mLocalFunction.template evaluate<Info::order, Info::ordering>(aDomain);
        tResult = aFunction(std::forward<Args>(aArgs)...);
    }
    constexpr auto tParentCommunicatorBroadcastRank = 0;
    return customization::split_parallel_synchronized_object(
        std::move(tResult), ParentCommunicator{aParentCommunicator}, SplitCommunicator{tSplitCommunicator},
        ParentBroadcastRank{tParentCommunicatorBroadcastRank}, GroupColor{tGroupColor.mValue});
}
*/
}  // namespace plato::utilities

#endif
