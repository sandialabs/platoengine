#ifndef PLATO_UTILTIES_MPIUTILITIES
#define PLATO_UTILTIES_MPIUTILITIES

#include <boost/mpi/collectives/broadcast.hpp>
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

/// @brief Groups an MPI communicator that was split from a parent communicator with its group color.
struct MPIGroupCommunicator
{
    boost::mpi::communicator mParentCommunicator;
    boost::mpi::communicator mSplitCommunicator;
    ColorNamedType mGroupColor;
};

/// @brief Given a parent communicator @a aParentCommunicator and a number of processors @a aNumberOfProcessors, splits
/// the parent communicator into two groups, the first with a number of ranks equal to @a aNumberOfProcessors, and the
/// second comprising the remaining ranks.
/// @pre @a aNumberOfProcessors must be less than or equal to the size of @a aParentCommunicator.
[[nodiscard]] auto group_split_communicator(const boost::mpi::communicator& aParentCommunicator,
                                            std::size_t aNumberOfProcessors) -> MPIGroupCommunicator;

/// @brief Computes a function using a subset of processors as defined in the split group in @a aGroupCommunicator. Only
/// the group with color 0 will evaluate the function, and the result is broadcast to all ranks on @a
/// aParentBroadcastRank.
/// @note @a aGroupCommunicator can be provided by group_split_communicator.
template <typename F, typename... Args>
    requires std::is_invocable_v<F, boost::mpi::communicator, Args...> &&
             std::is_default_constructible_v<std::invoke_result_t<F, boost::mpi::communicator, Args...>>
[[nodiscard]] auto compute_on_root_group(const MPIGroupCommunicator& aGroupCommunicator,
                                         int aParentBroadcastRank,
                                         const F& aFunction,
                                         Args&&... aArgs);

namespace customization
{
/// @brief Synchronizes data @a aObject between all ranks on @a aParentCommunicator.
///
/// It is assumed that the parent communicator in @a aGroupCommunicator was split into one or two groups and that one
/// entire group has a valid @a aObject. If the  split communicator is the same size as the parent communicator, it is
/// assumed that they are the same communicators and no synchronization is necessary. @a aParentBroadcastRoot is the
/// rank that will broadcast the object to all ranks. It is assumed that this rank has a valid object.
/// @note @a aGroupCommunicator can be provided by group_split_communicator.
/// @pre @a aParentBroadcastRoot is a rank that belongs to both the parent communicator and the split communicator.
/// @post All ranks return the same object @a aObject.
///
/// @note This function may also be used as a customization point for synchronizing function objects or other objects
/// not easily transmitted.
template <typename T>
[[nodiscard]] auto group_synchronized_object(T aObject,
                                             const MPIGroupCommunicator& aGroupCommunicator,
                                             int aParentBroadcastRank)
{
    if (aGroupCommunicator.mParentCommunicator.size() == aGroupCommunicator.mSplitCommunicator.size())
    {
        return aObject;
    }
    boost::mpi::broadcast(aGroupCommunicator.mParentCommunicator, aObject, aParentBroadcastRank);
    return aObject;
}
}  // namespace customization

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

template <typename F, typename... Args>
    requires std::is_invocable_v<F, boost::mpi::communicator, Args...> &&
             std::is_default_constructible_v<std::invoke_result_t<F, boost::mpi::communicator, Args...>>
auto compute_on_root_group(const MPIGroupCommunicator& aGroupCommunicator,
                           const int aParentBroadcastRank,
                           const F& aFunction,
                           Args&&... aArgs)
{
    using ReturnType = std::invoke_result_t<F, boost::mpi::communicator, Args...>;

    auto tResult = ReturnType{};
    if (aGroupCommunicator.mGroupColor.mValue == 0)
    {
        tResult = aFunction(aGroupCommunicator.mSplitCommunicator, std::forward<Args>(aArgs)...);
    }
    return customization::group_synchronized_object(std::move(tResult), aGroupCommunicator, aParentBroadcastRank);
}
}  // namespace plato::utilities

#endif
