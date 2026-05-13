#include <gtest/gtest.h>

#include <boost/mpi/communicator.hpp>
#include <ranges>

#include "plato/linear_algebra/JacobianMultiplier.hpp"
#include "plato/linear_algebra/ParallelJacobianMultiplier.hpp"
#include "plato/test_utilities/TestContext.hpp"

namespace plato::linear_algebra::parallel_unittest
{
namespace
{
[[nodiscard]] auto cube(const DynamicVector<double>& aDynamicVector) -> DynamicVector<double>
{
    auto tToCube = aDynamicVector.stdVector() | std::views::transform([](const auto& x) { return x * x * x; });
    return DynamicVector<double>{std::vector(tToCube.begin(), tToCube.end())};
}

template <typename MultiplierType>
void check_parallel_jacobian_multiplier(const MultiplierType& aMultiplierObject,
                                        const test_utilities::TestContext& aTestContext)
{
    const auto tParentCommunicator = boost::mpi::communicator{};
    for (const auto tNumberOfProcessorsForSplit : std::views::iota(1, tParentCommunicator.size() + 1))
    {
        const auto tGroupColor = tParentCommunicator.rank() <= tNumberOfProcessorsForSplit ? 0 : 1;
        const auto tSplitCommunicator = tParentCommunicator.split(tGroupColor);
        auto tMultiplierBase = MultiplierType{};
        if (tGroupColor == 0)
        {
            tMultiplierBase = aMultiplierObject;
        }
        constexpr auto tBroadcastRank = 0;
        tMultiplierBase = utilities::customization::group_synchronized_object(
            tMultiplierBase,
            utilities::MPIGroupCommunicator{.mParentCommunicator = tParentCommunicator,
                                            .mSplitCommunicator = tSplitCommunicator,
                                            .mGroupColor = utilities::ColorNamedType{tGroupColor}},
            tBroadcastRank);

        const auto tVector = DynamicVector{-1.0, 1.0, 2.0};
        const auto tExpected = DynamicVector{-1.0, 1.0, 8.0};
        const auto tResult = tVector * tMultiplierBase;
        EXPECT_EQ(tExpected.stdVector(), tResult.stdVector()) << aTestContext;
    }
}

}  // namespace

TEST(ParallelJacobianMultiplier, JacobianMultiplier)
{
    check_parallel_jacobian_multiplier<JacobianMultiplier>(
        JacobianMultiplier{.mVectorTimesJacobianFunction = [](const DynamicVector<double>& aRowVector)
                           { return cube(aRowVector); }},
        TEST_CONTEXT("Jacobian multiplier"));
}

TEST(ParallelJacobianMultiplier, AdjointJacobianMultiplier)
{
    check_parallel_jacobian_multiplier<AdjointJacobianMultiplier>(
        AdjointJacobianMultiplier{JacobianMultiplier{
            .mVectorTimesJacobianFunction = [](const DynamicVector<double>& aRowVector) { return cube(aRowVector); }}},
        TEST_CONTEXT("Jacobian multiplier"));
}
}  // namespace plato::linear_algebra::parallel_unittest
