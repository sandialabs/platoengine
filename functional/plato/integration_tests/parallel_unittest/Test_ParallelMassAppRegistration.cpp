#include <gtest/gtest.h>

#include <boost/mpi/communicator.hpp>

#include "plato/integration_tests/utilities/MassAppTestUtilities.hpp"

namespace plato::integration_tests::parallel
{

TEST(ParallelMassAppRegistration, RegisterLoadAndRun)
{
    const auto tComm = boost::mpi::communicator{};
    EXPECT_GT(tComm.size(), 1u);
    utilities::register_load_run_test(tComm, TEST_CONTEXT("Parallel mass app test"));
}
}  // namespace plato::integration_tests::parallel