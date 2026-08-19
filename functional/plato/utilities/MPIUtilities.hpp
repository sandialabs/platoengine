#ifndef PLATO_UTILTIES_MPIUTILITIES
#define PLATO_UTILTIES_MPIUTILITIES

#include <boost/mpi/collectives/broadcast.hpp>
#include <boost/mpi/communicator.hpp>
#include <format>
#include <utility>

#include "plato/utilities/Exception.hpp"
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

/// @brief Helper variable template for checking if a function has a value-type return.
template <typename Function, typename... Args>
constexpr inline auto kHasValueTypeReturn =
    std::is_invocable_v<Function, Args...> && !std::is_void_v<std::invoke_result_t<Function, Args...>> &&
    !std::is_reference_v<std::invoke_result_t<Function, Args...>>;

/// @brief Computes a function using a subset of processors as defined in the split group in @a aGroupCommunicator. Only
/// the group with color 0 will evaluate the function, and the result is broadcast to all ranks on @a
/// aParentBroadcastRank.
/// @note @a aGroupCommunicator can be provided by group_split_communicator.
template <typename F, typename... Args>
    requires kHasValueTypeReturn<F, const boost::mpi::communicator&, Args...> &&
             std::is_default_constructible_v<std::invoke_result_t<F, const boost::mpi::communicator&, Args...>>
[[nodiscard]] auto compute_on_root_group(const MPIGroupCommunicator& aGroupCommunicator,
                                         int aParentBroadcastRank,
                                         const F& aFunction,
                                         Args&&... aArgs);

/// @brief Executes @a aOperation on all ranks inside a `try/catch` block. If any rank throws, this will then throw
/// on all ranks, though the exception type may be different.
///
/// @note This can be used to handle the case in which some subset of ranks in a collective operation may throw.
///  When that happens, any subsequent blocking MPI operations may result in a hang since some ranks will be processing
///  exceptions and some will advance as normal.
/// @note This is only implemented for value-type return types. It is not implemented for `void` or reference-types.
///  Returning reference-types requires some additional care since some ranks may not return anything if they throw.
template <typename Function, typename... Args>
    requires kHasValueTypeReturn<Function, Args...>
[[nodiscard]] auto mpi_collective_try(const boost::mpi::communicator& aCommunicator,
                                      const Function& aOperation,
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
    requires kHasValueTypeReturn<F, const boost::mpi::communicator&, Args...> &&
             std::is_default_constructible_v<std::invoke_result_t<F, const boost::mpi::communicator&, Args...>>
auto compute_on_root_group(const MPIGroupCommunicator& aGroupCommunicator,
                           const int aParentBroadcastRank,
                           const F& aFunction,
                           Args&&... aArgs)
{
    using ReturnType = std::invoke_result_t<F, boost::mpi::communicator, Args...>;

    auto tResult = mpi_collective_try(
        aGroupCommunicator.mParentCommunicator,
        [](const MPIGroupCommunicator& aGroupCommunicator, const F& aFunction, auto&&... aArgs) -> ReturnType
        {
            auto tResult = ReturnType{};
            if (aGroupCommunicator.mGroupColor.mValue == 0)
            {
                tResult = aFunction(aGroupCommunicator.mSplitCommunicator, std::forward<Args>(aArgs)...);
            }
            return tResult;
        },
        aGroupCommunicator, aFunction, std::forward<Args>(aArgs)...);
    return customization::group_synchronized_object(std::move(tResult), aGroupCommunicator, aParentBroadcastRank);
}

template <typename Function, typename... Args>
    requires kHasValueTypeReturn<Function, Args...>
auto mpi_collective_try(const boost::mpi::communicator& aCommunicator, const Function& aOperation, Args&&... aArgs)
{
    using ReturnType = decltype(aOperation(std::forward<Args>(aArgs)...));

    auto tExceptionMessage = std::optional<std::string>{};
    auto tFunctionReturn = std::optional<ReturnType>{};
    try
    {
        tFunctionReturn = aOperation(std::forward<Args>(aArgs)...);
    }
    catch (const std::exception& tException)
    {
        const auto tRank = aCommunicator.rank();
        tExceptionMessage =
            std::format("Encountered an exception on rank {} with message: {}\n", tRank, tException.what());
    }
    catch (...)
    {
        const auto tRank = aCommunicator.rank();
        tExceptionMessage =
            std::format("Encountered an uknown exception in an MPI collective operation on rank {}.", tRank);
    }

    if (boost::mpi::all_reduce(aCommunicator, tExceptionMessage.has_value(), std::logical_or{}))
    {
        const auto tNewExceptionMessage = tExceptionMessage.value_or(
            std::string{"Another rank encountered an exception in an MPI collective operation."});
        throw Exception{tNewExceptionMessage};
    }

    return std::move(tFunctionReturn).value();
}

}  // namespace plato::utilities

#endif
