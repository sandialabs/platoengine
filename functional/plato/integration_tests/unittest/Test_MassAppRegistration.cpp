#include <gtest/gtest.h>

#include <boost/mpi/communicator.hpp>

#include "plato/integration_tests/utilities/MassAppTestUtilities.hpp"

namespace plato::integration_tests::serial
{
namespace
{
}  // namespace

TEST(MassAppRegistration, RegisterLoadAndRun)
{
    const auto tComm = boost::mpi::communicator{};
    EXPECT_EQ(tComm.size(), 1u);
    utilities::register_load_run_test(tComm, TEST_CONTEXT("Serial mass app test"));
}
}  // namespace plato::integration_tests::serial