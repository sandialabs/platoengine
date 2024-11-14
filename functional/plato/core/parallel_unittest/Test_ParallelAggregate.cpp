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
constexpr auto kNumRanks = int{2};

using Domain = const plato::test_utilities::TwoDVector&;
using EvaluationInfo = FunctionInfo<double, evaluation::kFunction>;
using GradientInfo = FunctionInfo<plato::test_utilities::TwoDVector, evaluation::kFirstDerivative>;
}  // namespace

TEST(ParallelAggregate, MPISize)
{
    auto tComm = boost::mpi::communicator{};
    EXPECT_EQ(tComm.size(), kNumRanks);
}

TEST(ParallelAggregate, EvaluateSame)
{
    namespace pft = plato::test_utilities;

    // This assumes this test is running in parallel w/ `kNumRanks` number of ranks.
    // Each rank constructs a `ParallelAggregate` object with a single function, so that
    // the resulting aggregation should be the evaluation of one function times the number
    // of ranks.
    const auto tF = test_utilities::make_two_d_function(pft::Rosenbrock{});
    const double tW = 0.5;
    using RosenbrockF = std::decay_t<decltype(tF)>;
    using FunctionAndWeight = std::vector<std::pair<RosenbrockF, double>>;
    const auto tAggregate = ParallelAggregate<Domain, EvaluationInfo, GradientInfo>(
        FunctionAndWeight{std::make_pair(tF, tW)}, boost::mpi::communicator{});

    EXPECT_EQ(tAggregate.size(), 1);

    const auto tArg = pft::TwoDVector{2.0, -1.0};
    const double tExpectedF = kNumRanks * tW * tF.evaluate<evaluation::kFunction>(tArg);
    EXPECT_EQ(tAggregate.evaluate<evaluation::kFunction>(tArg), tExpectedF);
    const pft::TwoDVector tExpectedDF = kNumRanks * tW * tF.evaluate<evaluation::kFirstDerivative>(tArg);
    EXPECT_EQ(tAggregate.evaluate<evaluation::kFirstDerivative>(tArg), tExpectedDF);
}

TEST(ParallelAggregate, EvaluateDifferent)
{
    namespace pft = plato::test_utilities;

    const auto tCommunicator = boost::mpi::communicator{};
    constexpr double tA = 2.0;
    constexpr double tB = 50.0;
    const auto tFNonDefault = test_utilities::make_two_d_function(pft::Rosenbrock{tA, tB});
    const auto tFDefault = test_utilities::make_two_d_function(pft::Rosenbrock{});

    using RosenbrockF = std::decay_t<decltype(tFNonDefault)>;
    using FunctionAndWeight = std::vector<std::pair<RosenbrockF, double>>;
    constexpr auto tW = double{0.5};
    const auto tFunctionAndWeight = tCommunicator.rank() == 0 ? FunctionAndWeight{std::make_pair(tFDefault, tW)}
                                                              : FunctionAndWeight{std::make_pair(tFNonDefault, tW)};
    const auto tAggregate = ParallelAggregate<Domain, EvaluationInfo, GradientInfo>(tFunctionAndWeight, tCommunicator);

    EXPECT_EQ(tAggregate.size(), 1);

    const auto tArg = pft::TwoDVector{2.0, -1.0};
    const double tExpectedF =
        tW * (tFNonDefault.evaluate<evaluation::kFunction>(tArg) + tFDefault.evaluate<evaluation::kFunction>(tArg));
    EXPECT_EQ(tAggregate.evaluate<evaluation::kFunction>(tArg), tExpectedF);
    const pft::TwoDVector tExpectedDF = tW * (tFNonDefault.evaluate<evaluation::kFirstDerivative>(tArg) +
                                              tFDefault.evaluate<evaluation::kFirstDerivative>(tArg));
    EXPECT_EQ(tAggregate.evaluate<evaluation::kFirstDerivative>(tArg), tExpectedDF);
}
}  // namespace plato::core::parallel_unittest
