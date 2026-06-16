#include <gtest/gtest.h>

#include <boost/mpi/communicator.hpp>
#include <functional>
#include <ranges>

#include "plato/utilities/MPIUtilities.hpp"

namespace plato::utilities::parallel_unittest
{

TEST(MPIUtilities, ExecuteOnRoot)
{
    constexpr auto tValue = 666;
    constexpr auto tOriginal = 42;

    const auto tLambda = [](int& aInteger) { aInteger = tValue; };
    const auto tCommunicator = boost::mpi::communicator{};

    auto tInteger = tOriginal;
    execute_on_root(tCommunicator, tLambda, tInteger);

    if (tCommunicator.rank() == 0)
    {
        EXPECT_EQ(tInteger, tValue);
    }
    else
    {
        EXPECT_EQ(tInteger, tOriginal);
    }
}

TEST(MPIUtilities, GroupSplitCommunicator)
{
    const auto tParentCommunicator = boost::mpi::communicator{};
    for (const auto tNumberOfProcessors : std::views::iota(0, tParentCommunicator.size() + 1))
    {
        const auto [tParentCommunicatorFromGroup, tGroupCommunicator, tGroupColor] =
            group_split_communicator(tParentCommunicator, tNumberOfProcessors);

        EXPECT_EQ(tParentCommunicatorFromGroup, tParentCommunicator);

        if (tParentCommunicator.rank() < tNumberOfProcessors)
        {
            EXPECT_EQ(tGroupColor.mValue, 0U);
            EXPECT_EQ(tGroupCommunicator.size(), tNumberOfProcessors);
        }
        else
        {
            EXPECT_EQ(tGroupColor.mValue, 1U);
            EXPECT_EQ(tGroupCommunicator.size(), tParentCommunicator.size() - tNumberOfProcessors);
        }
    }
}

TEST(MPIUtilities, ComputeOnRootGroup)
{
    const auto tParentCommunicator = boost::mpi::communicator{};
    for (const auto tNumberOfProcessors : std::views::iota(0, tParentCommunicator.size() + 1))
    {
        const auto tGroupCommunicator = group_split_communicator(tParentCommunicator, tNumberOfProcessors);

        constexpr auto tGroupRootColor = 0;
        using FunctionType = std::function<int(const boost::mpi::communicator&)>;
        const auto tFunction =
            (tGroupCommunicator.mGroupColor.mValue == tGroupRootColor)
                ? FunctionType{[](const boost::mpi::communicator& aCommunicator) { return aCommunicator.size(); }}
                : FunctionType{};

        constexpr auto tParentCommunicatorBroadcastRank = 0;
        const auto tResult = compute_on_root_group(tGroupCommunicator, tParentCommunicatorBroadcastRank, tFunction);

        EXPECT_EQ(tResult, tNumberOfProcessors) << "Failure for " << tNumberOfProcessors << " processors";
    }
}

TEST(MPIUtilities, GroupSynchronizedDouble)
{
    const auto tParentCommunicator = boost::mpi::communicator{};
    for (const auto tGroupSize : std::views::iota(1, tParentCommunicator.size() + 1))
    {
        const auto tGroupColor = (tParentCommunicator.rank() < tGroupSize) ? 0 : 1;
        const auto tSplitCommunicator = tParentCommunicator.split(tGroupColor);

        const auto tPayload = 67.0;
        constexpr auto tParentCommunicatorBroadcastRank = 0;
        const auto tReceivedPayload =
            customization::group_synchronized_object(tPayload,
                                                     MPIGroupCommunicator{.mParentCommunicator = tParentCommunicator,
                                                                          .mSplitCommunicator = tSplitCommunicator,
                                                                          .mGroupColor = ColorNamedType{tGroupColor}},
                                                     tParentCommunicatorBroadcastRank);
        EXPECT_EQ(tReceivedPayload, tPayload);
    }
}

}  // namespace plato::utilities::parallel_unittest
