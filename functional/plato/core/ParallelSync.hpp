#ifndef PLATO_CORE_PARALLELSYNC
#define PLATO_CORE_PARALLELSYNC

#include <boost/mpi/collectives/broadcast.hpp>
#include <boost/mpi/communicator.hpp>

#include "plato/utilities/NamedType.hpp"

namespace plato::core
{
using ParentCommunicator = utilities::NamedType<boost::mpi::communicator, struct ParentCommunicatorTag>;
using SplitCommunicator = utilities::NamedType<boost::mpi::communicator, struct SplitCommunicatorTag>;
using ParentBroadcastRank = utilities::NamedType<int, struct ParentBroadcastRankTag>;
using GroupColor = utilities::NamedType<int, struct GroupColorTag>;

/// @brief Syncs data @a aObject between all ranks on @a aParentCommunicator.
///
/// It is assumed that @a aParentCommunicator was split into one or two groups and that one entire group has a valid @a
/// aObject. If @a aSplitCommunicator is the same size as @a aParentCommunicator, it is assumed that they are the same
/// communicators and no synchronization is necessary. @a aParentBroadcastRoot is the rank that will broadcast the
/// object to all ranks. It is assumed that this rank has a valid object.
/// @pre @a aParentBroadcastRoot is a rank that belongs to both the parent communicator and the split communicator.
/// @post All ranks return the same object @a aObject.
///
/// @note This function may also be used as a customization point for synchronizing function objects or other objects
/// not easily transmitted.
template <typename T>
[[nodiscard]] auto parallel_sync(T aObject,
                                 const ParentCommunicator& aParentCommunicator,
                                 const SplitCommunicator& aSplitCommunicator,
                                 const ParentBroadcastRank aParentBroadcastRank,
                                 const GroupColor /*aGroupColor*/)
{
    if (aParentCommunicator.mValue.size() == aSplitCommunicator.mValue.size())
    {
        return aObject;
    }
    boost::mpi::broadcast(aParentCommunicator.mValue, aObject, aParentBroadcastRank.mValue);
    return aObject;
}

}  // namespace plato::core

#endif
