#include <gtest/gtest.h>

#include <boost/mpi/communicator.hpp>
#include <ranges>

#include "plato/core/ParallelSync.hpp"

namespace plato::core::unittest
{
TEST(ParallelSync, ParallelSyncDouble)
{
    const auto tParentCommunicator = boost::mpi::communicator{};
    for (const auto tGroupSize : std::views::iota(1, tParentCommunicator.size() + 1))
    {
        const auto tGroupColor = tParentCommunicator.rank() < tGroupSize;
        const auto tSplitCommunicator = tParentCommunicator.split(tGroupColor);

        const auto tPayload = 67.0;
        constexpr auto tParentCommunicatorBroadcastRank = 0;
        const auto tReceivedPayload =
            parallel_sync(tPayload, ParentCommunicator{tParentCommunicator}, SplitCommunicator{tSplitCommunicator},
                          ParentBroadcastRank{tParentCommunicatorBroadcastRank}, GroupColor{tGroupColor});
        EXPECT_EQ(tReceivedPayload, tPayload);
    }
}
}  // namespace plato::core::unittest
