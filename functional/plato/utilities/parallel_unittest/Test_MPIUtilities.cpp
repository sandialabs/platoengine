#include <gtest/gtest.h>

#include <boost/mpi/communicator.hpp>
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
        const auto [tGroupCommunicator, tGroupColor] =
            group_split_communicator(tParentCommunicator, tNumberOfProcessors);
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
}  // namespace plato::utilities::parallel_unittest
