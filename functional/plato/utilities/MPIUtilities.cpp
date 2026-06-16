#include "plato/utilities/MPIUtilities.hpp"

#include <boost/mpi/communicator.hpp>
#include <cassert>
#include <cstddef>
#include <vector>

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
}  // namespace plato::utilities
