#include <gtest/gtest.h>

#include <boost/log/attributes/constant.hpp>
#include <boost/mpi/communicator.hpp>

#include "plato/third_party_integration/boost_log/MPIAttributes.hpp"
#include "plato/third_party_integration/boost_log/test_utilities/TestUtilities.hpp"

namespace plato::third_party_integration::boost_log::parallel_unittest
{
TEST(MPIAttributes, MPIRootRankFilter)
{
    const auto tWorldCommunicator = boost::mpi::communicator{};
    const auto tMPIRootRankFilter = MPIRankAttribute::filter();
    const auto tMPIRankAttribute = test_utilities::attribute_set(MPIRankAttribute::name(), tWorldCommunicator.rank());
    const auto tAttributeValueSet = test_utilities::attribute_value_set(tMPIRankAttribute);

    const auto tExpected = tWorldCommunicator.rank() == 0;
    EXPECT_EQ(tMPIRootRankFilter(tAttributeValueSet), tExpected);
}
}  // namespace plato::third_party_integration::boost_log::parallel_unittest
