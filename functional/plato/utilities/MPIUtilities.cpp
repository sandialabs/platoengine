#include "plato/utilities/MPIUtilities.hpp"

#include <boost/mpi/collectives/all_reduce.hpp>
#include <boost/mpi/communicator.hpp>
#include <cassert>
#include <cstddef>
#include <exception>
#include <format>
#include <functional>
#include <optional>
#include <string>
#include <vector>

#include "plato/utilities/Exception.hpp"
#include "plato/utilities/RankSplitVector.hpp"

namespace plato::utilities
{
auto group_split_communicator(const boost::mpi::communicator& aParentCommunicator,
                              const std::size_t aNumberOfProcessors) -> MPIGroupCommunicator
{
    assert(aNumberOfProcessors <= static_cast<std::size_t>(aParentCommunicator.size()));

    const auto tGroupSizes = std::vector{static_cast<unsigned int>(aNumberOfProcessors),
                                         static_cast<unsigned int>(aParentCommunicator.size() - aNumberOfProcessors)};
    const auto tGroupColor =
        utilities::rank_group_color(tGroupSizes, utilities::RankNamedType{aParentCommunicator.rank()});
    const auto tSplitCommunicator = aParentCommunicator.split(tGroupColor.mValue);
    return {.mParentCommunicator = aParentCommunicator,
            .mSplitCommunicator = tSplitCommunicator,
            .mGroupColor = tGroupColor};
}

namespace detail
{
auto exception_message(const int aRank, const std::exception& aException) -> std::string
{
    return std::format("Encountered an exception on rank {} with message: {}\n", aRank, aException.what());
}
auto exception_message(const int aRank) -> std::string
{
    return std::format("Encountered an uknown exception in an MPI collective operation on rank {}.", aRank);
}

void handle_parallel_exception(const boost::mpi::communicator& aCommunicator,
                               const std::optional<std::string>& aExceptionMessage)
{
    if (boost::mpi::all_reduce(aCommunicator, aExceptionMessage.has_value(), std::logical_or{}))
    {
        const auto tNewExceptionMessage = aExceptionMessage.value_or(
            std::string{"Another rank encountered an exception in an MPI collective operation."});
        throw Exception{tNewExceptionMessage};
    }
}

}  // namespace detail
}  // namespace plato::utilities
