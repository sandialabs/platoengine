#include <gtest/gtest.h>

#include <boost/mpi/communicator.hpp>

#include "plato/core/ParallelAggregate.hpp"
#include "plato/core/ParallelFunction.hpp"
#include "plato/integration_tests/utilities/DynamicVectorRosenbrockFunction.hpp"
#include "plato/linear_algebra/DynamicVector.hpp"
#include "plato/linear_algebra/DynamicVectorSerialization.hpp"
#include "plato/test_utilities/ParallelFunction.hpp"
#include "plato/test_utilities/ParallelTestWrapper.hpp"
#include "plato/test_utilities/Rosenbrock.hpp"
#include "plato/test_utilities/TwoDTestTypesToDynamicVector.hpp"

namespace plato::integration_tests::parallel
{
TEST(ParallelAggregateDynamicVector, ParallelAggregateOneRosenbrockPerRank)
{
    namespace ptu = plato::test_utilities;

    // This assumes this test is running in parallel w/ `kNumRanks` number of ranks.
    // Each rank constructs a `ParallelAggregate` object with a single function, so that
    // the resulting aggregation should be the evaluation of one function times the number
    // of ranks.
    const auto tRosenbrockFunction = utilities::make_rosenbrock_dynamic_vector_function(ptu::Rosenbrock{});
    using RosenbrockF = std::decay_t<decltype(tRosenbrockFunction)>;
    constexpr auto tWeight = double{0.5};

    using FunctionAndWeight = std::vector<std::pair<RosenbrockF, double>>;
    const auto tWorld = boost::mpi::communicator{};
    const auto tAggregate = core::ParallelAggregate<double, linear_algebra::DynamicVector<double>,
                                                    const linear_algebra::DynamicVector<double>&>(
        FunctionAndWeight{std::make_pair(tRosenbrockFunction, tWeight)}, tWorld);

    const auto tNumRanks = tWorld.size();
    ASSERT_GT(tNumRanks, 1);

    const auto tControl = linear_algebra::DynamicVector{1.0, -2.0};
    const double tExpectedF = tNumRanks * tWeight * tRosenbrockFunction.f(tControl);
    const double tComputedF = tAggregate.f(tControl);
    EXPECT_EQ(tComputedF, tExpectedF);
    const linear_algebra::DynamicVector<double> tExpectedDF = tNumRanks * tWeight * tRosenbrockFunction.df(tControl);
    const linear_algebra::DynamicVector<double> tComputedDF = tAggregate.df(tControl);
    EXPECT_EQ(tComputedDF, tExpectedDF);
}

TEST(ParallelAggregateDynamicVector, ParallelAggregateParallelRosenbrock)
{
    // This test is similar to the above, but uses two Rosenbrock objectives that run in parallel
    // in two groups. So the parallelization is over the Rosenbrock functions and their sum.
    namespace ptu = plato::test_utilities;
    using GradientType = linear_algebra::DynamicVector<double>;
    using ArgType = linear_algebra::DynamicVector<double>;

    // Make mpi groups by even and odd ranks
    const auto tWorld = boost::mpi::communicator{};
    constexpr auto kNumGroups = 3;
    const auto tComm = tWorld.split(tWorld.rank() % kNumGroups);

    // Make a phony parallel Rosenbrock
    const auto tRosenbrock = ptu::Rosenbrock{};
    const auto tParallelRosenbrock = ptu::ParallelTestFunctionWrapper<double, const ArgType&>{
        [tRosenbrock = tRosenbrock](const ArgType& x) { return tRosenbrock.f(x[0], x[1]); }};
    const auto tDerivativeParallelRosenbrock = ptu::ParallelTestFunctionWrapper<GradientType, const ArgType&>{
        [tRosenbrock = tRosenbrock](const ArgType& x) { return ptu::to_dynamic_vector(tRosenbrock.df(x[0], x[1])); }};

    const auto tParallelFunction =
        ptu::make_parallel_function(tParallelRosenbrock, tDerivativeParallelRosenbrock, tComm);
    const auto tAdaptedParallelFunction = core::adapt_parallel_function(tParallelFunction, tComm);

    // Use in parallel aggregate
    using RosenbrockF = std::decay_t<decltype(tAdaptedParallelFunction)>;
    using FunctionAndWeight = std::vector<std::pair<RosenbrockF, double>>;
    constexpr auto tWeight = double{0.5};
    const auto tAggregate = core::ParallelAggregate<double, GradientType, const ArgType&>(
        FunctionAndWeight{std::make_pair(tAdaptedParallelFunction, tWeight)}, tWorld);

    const auto tControl = linear_algebra::DynamicVector{1.0, -2.0};

    const auto tNumRanks = tWorld.size();
    ASSERT_GT(tNumRanks, 3) << "Test uses three groups and so it requires at least three ranks.";

    const auto tExpectedF = kNumGroups * tWeight * tRosenbrock.f(tControl[0], tControl[1]);
    EXPECT_EQ(tAggregate.f(tControl), tExpectedF);
}

}  // namespace plato::integration_tests::parallel
