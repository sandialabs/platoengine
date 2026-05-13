#include <gtest/gtest.h>

#include <boost/mpi/collectives/broadcast.hpp>
#include <boost/mpi/communicator.hpp>
#include <ranges>

#include "plato/core/Function.hpp"
#include "plato/core/ParallelFunction.hpp"
#include "plato/core/SplitParallelFunction.hpp"
#include "plato/test_utilities/Rosenbrock.hpp"
#include "plato/test_utilities/TwoDTestTypesSerialization.hpp"

namespace plato::core::parallel_unittest
{
namespace
{
constexpr auto kRosenbrockA = double{1.0};
constexpr auto kRosenbrockB = double{10.0};
constexpr auto kRosenbrock = plato::test_utilities::Rosenbrock{kRosenbrockA, kRosenbrockB};

/// @brief Returns a Function that computes the Rosenbrock function using a communicator for testing parallelization.
///
/// Only the root rank computes the function, and then it is broadcast to the other ranks on the communicator.
[[nodiscard]] auto parallel_rosenbrock(const boost::mpi::communicator& aCommunicator)
{
    const auto tF = [aCommunicator](const plato::test_utilities::TwoDVector& aVector)
    {
        if (aCommunicator.rank() == 0U)
        {
            const auto tRosenbrockEvaluation = kRosenbrock.f(aVector.mData[0], aVector.mData[1]);
            return broadcast_from_root(aCommunicator, tRosenbrockEvaluation);
        }
        return broadcast_from_root(aCommunicator, 0.0);
    };

    const auto tDF = [aCommunicator](const plato::test_utilities::TwoDVector& aVector)
    {
        if (aCommunicator.rank() == 0U)
        {
            const auto tDerivative = kRosenbrock.df(aVector.mData[0], aVector.mData[1]);
            return broadcast_from_root(aCommunicator, tDerivative);
        }
        return broadcast_from_root(aCommunicator, test_utilities::TwoDVector{});
    };
    return make_function_with_first_derivative(tF, tDF);
}
}  // namespace

TEST(SplitParallelFunction, Evaluate)
{
    const auto tParentCommunicator = boost::mpi::communicator{};
    for (const auto tNumberOfProcessorsForSplit : std::views::iota(1, tParentCommunicator.size() + 1))
    {
        const auto tSplitFunction = make_split_parallel_function(tParentCommunicator, tNumberOfProcessorsForSplit,
                                                                 [](const boost::mpi::communicator& aGroupCommunicator)
                                                                 { return parallel_rosenbrock(aGroupCommunicator); });

        constexpr auto tEvaluationPoint = test_utilities::TwoDVector{-1.0, 2.0};
        const auto tFunctionEvaluation = tSplitFunction.evaluate<evaluation::kFunction>(tEvaluationPoint);
        constexpr auto tExpectedFunctionEvaluation =
            kRosenbrock.f(tEvaluationPoint.mData[0], tEvaluationPoint.mData[1]);

        EXPECT_DOUBLE_EQ(tFunctionEvaluation, tExpectedFunctionEvaluation)
            << "Function evaluation, number of ranks: " << tNumberOfProcessorsForSplit;

        const auto tGradientEvaluation = tSplitFunction.evaluate<evaluation::kFirstDerivative>(tEvaluationPoint);
        constexpr auto tExpectedGradient = kRosenbrock.df(tEvaluationPoint.mData[0], tEvaluationPoint.mData[1]);
        EXPECT_EQ(tGradientEvaluation, tExpectedGradient)
            << "Gradient evaluation number of ranks: " << tNumberOfProcessorsForSplit;
    }
}

}  // namespace plato::core::parallel_unittest
