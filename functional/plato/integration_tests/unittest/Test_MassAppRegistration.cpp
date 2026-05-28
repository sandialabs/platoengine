#include <gtest/gtest.h>

#include <boost/mpi/communicator.hpp>

#include "plato/integration_tests/utilities/MassAppTestUtilities.hpp"
#include "plato/test_utilities/TestContext.hpp"

namespace plato::integration_tests::serial
{
namespace
{
}  // namespace

TEST(MassAppRegistration, RegisterLoadAndRun)
{
    const auto tCommunicator = boost::mpi::communicator{};
    EXPECT_EQ(tCommunicator.size(), 1U);
    utilities::register_load_run_test(tCommunicator, TEST_CONTEXT("Serial mass app test"));
}
}  // namespace plato::integration_tests::serial