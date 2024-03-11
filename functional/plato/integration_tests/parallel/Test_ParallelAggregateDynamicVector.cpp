#include <gtest/gtest.h>

#include <boost/mpi/communicator.hpp>

#include "plato/core/ParallelAggregate.hpp"
#include "plato/integration_tests/utilities/DynamicVectorRosenbrockFunction.hpp"
#include "plato/linear_algebra/DynamicVector.hpp"
#include "plato/linear_algebra/DynamicVectorSerialization.hpp"
#include "plato/test_utilities/Rosenbrock.hpp"

namespace plato::integration_tests::parallel
{
namespace
{
constexpr auto kNumRanks = int{4};
}

TEST(ParallelAggregateDynamicVector, MPISize)
{
    auto tComm = boost::mpi::communicator{};
    EXPECT_EQ(tComm.size(), kNumRanks);
}

TEST(ParallelAggregateDynamicVector, ParallelAggregateTwoRosenbrockObjectives)
{
    namespace pft = plato::test_utilities;

    // This assumes this test is running in parallel w/ `kNumRanks` number of ranks.
    // Each rank constructs a `ParallelAggregate` object with a single function, so that
    // the resulting aggregation should be the evaluation of one function times the number
    // of ranks.
    const auto tRosenbrockFunction = utilities::make_rosenbrock_dynamic_vector_function(pft::Rosenbrock{});
    using RosenbrockF = std::decay_t<decltype(tRosenbrockFunction)>;
    constexpr auto tWeight = double{0.5};

    using FunctionAndWeight = std::vector<std::pair<RosenbrockF, double>>;
    const auto tAggregate = core::ParallelAggregate<double, linear_algebra::DynamicVector<double>,
                                                    const linear_algebra::DynamicVector<double>&>(
        FunctionAndWeight{std::make_pair(tRosenbrockFunction, tWeight)}, boost::mpi::communicator{});

    const auto tControl = linear_algebra::DynamicVector{1.0, -2.0};
    const double tExpectedF = kNumRanks * tWeight * tRosenbrockFunction.f(tControl);
    const double tComputedF = tAggregate.f(tControl);
    EXPECT_EQ(tComputedF, tExpectedF);
    const linear_algebra::DynamicVector<double> tExpectedDF = kNumRanks * tWeight * tRosenbrockFunction.df(tControl);
    const linear_algebra::DynamicVector<double> tComputedDF = tAggregate.df(tControl);
    EXPECT_EQ(tComputedDF, tExpectedDF);
}
}  // namespace plato::integration_tests::parallel
