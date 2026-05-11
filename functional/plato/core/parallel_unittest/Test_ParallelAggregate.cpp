#include <gtest/gtest.h>

#include <boost/mpi/communicator.hpp>

#include "plato/core/ParallelAggregate.hpp"
#include "plato/core/test_utilities/Utilities.hpp"
#include "plato/test_utilities/Rosenbrock.hpp"
#include "plato/test_utilities/TwoDTestTypesSerialization.hpp"

namespace plato::core::parallel_unittest
{
namespace
{
using Domain = const plato::test_utilities::TwoDVector&;
using EvaluationInfo = FunctionInfo<double, evaluation::kFunction>;
using GradientInfo = FunctionInfo<plato::test_utilities::TwoDVector, evaluation::kFirstDerivative>;
}  // namespace

TEST(ParallelAggregate, EvaluateSame)
{
    namespace pft = plato::test_utilities;

    const auto tNumberOfRanks = boost::mpi::communicator{}.size();
    if (tNumberOfRanks <= 1)
    {
        GTEST_SKIP() << "Test requires more than 1 rank.";
    }
    // This assumes this test is running in parallel w/ `kNumRanks` number of ranks.
    // Each rank constructs a `ParallelAggregate` object with a single function, so that
    // the resulting aggregation should be the evaluation of one function times the number
    // of ranks.
    const auto tRosenbrockFunction = test_utilities::make_two_d_function(pft::Rosenbrock{});
    const auto tWeight = 1.0 / static_cast<double>(tNumberOfRanks);
    using RosenbrockF = std::decay_t<decltype(tRosenbrockFunction)>;
    using FunctionAndWeight = std::vector<std::pair<RosenbrockF, double>>;
    const auto tAggregate = ParallelAggregate<Domain, EvaluationInfo, GradientInfo>(
        FunctionAndWeight{std::make_pair(tRosenbrockFunction, tWeight)}, boost::mpi::communicator{});

    EXPECT_EQ(tAggregate.size(), 1);

    const auto tFunctionArgument = pft::TwoDVector{2.0, -1.0};
    const auto tExpectedFunctionEvaluation = tRosenbrockFunction.evaluate<evaluation::kFunction>(tFunctionArgument);
    EXPECT_EQ(tAggregate.evaluate<evaluation::kFunction>(tFunctionArgument), tExpectedFunctionEvaluation);
    const auto tExpectedDF = tRosenbrockFunction.evaluate<evaluation::kFirstDerivative>(tFunctionArgument);
    EXPECT_EQ(tAggregate.evaluate<evaluation::kFirstDerivative>(tFunctionArgument), tExpectedDF);
}

TEST(ParallelAggregate, EvaluateDifferent)
{
    namespace pft = plato::test_utilities;

    const auto tCommunicator = boost::mpi::communicator{};
    const auto tNumberOfRanks = tCommunicator.size();
    if (tNumberOfRanks <= 1)
    {
        GTEST_SKIP() << "Test requires more than 1 rank.";
    }

    constexpr double tA = 2.0;
    constexpr double tB = 50.0;
    const auto tRosenbrockFunctionNonDefault = test_utilities::make_two_d_function(pft::Rosenbrock{tA, tB});
    const auto tRosenbrockFunctionDefault = test_utilities::make_two_d_function(pft::Rosenbrock{});

    using RosenbrockF = std::decay_t<decltype(tRosenbrockFunctionNonDefault)>;
    using FunctionAndWeight = std::vector<std::pair<RosenbrockF, double>>;
    constexpr auto tWeight = double{0.5};
    const auto tRosenbrockFunctionunctionAndWeight =
        tCommunicator.rank() == 0 ? FunctionAndWeight{std::make_pair(tRosenbrockFunctionDefault, tWeight)}
                                  : FunctionAndWeight{std::make_pair(tRosenbrockFunctionNonDefault, tWeight)};
    const auto tAggregate =
        ParallelAggregate<Domain, EvaluationInfo, GradientInfo>(tRosenbrockFunctionunctionAndWeight, tCommunicator);

    EXPECT_EQ(tAggregate.size(), 1);

    const auto tFunctionArgument = pft::TwoDVector{2.0, -1.0};
    const double tExpectedFunctionEvaluation =
        tWeight *
        ((tNumberOfRanks - 1) * tRosenbrockFunctionNonDefault.evaluate<evaluation::kFunction>(tFunctionArgument) +
         tRosenbrockFunctionDefault.evaluate<evaluation::kFunction>(tFunctionArgument));
    EXPECT_EQ(tAggregate.evaluate<evaluation::kFunction>(tFunctionArgument), tExpectedFunctionEvaluation);

    const pft::TwoDVector tExpectedDF =
        tWeight * ((tNumberOfRanks - 1) *
                       tRosenbrockFunctionNonDefault.evaluate<evaluation::kFirstDerivative>(tFunctionArgument) +
                   tRosenbrockFunctionDefault.evaluate<evaluation::kFirstDerivative>(tFunctionArgument));
    EXPECT_EQ(tAggregate.evaluate<evaluation::kFirstDerivative>(tFunctionArgument), tExpectedDF);
}
}  // namespace plato::core::parallel_unittest
