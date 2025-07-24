#include <gtest/gtest.h>

#include <boost/log/attributes/constant.hpp>
#include <boost/mpi/communicator.hpp>

#include "plato/third_party_integration/boost_log/MPIAttributes.hpp"

namespace plato::third_party_integration::boost_log::parallel_unittest
{
TEST(MPIAttributes, MPIRootRankFilter)
{
    const auto tWorldCommunicator = boost::mpi::communicator{};
    const auto tMPIRootRankFilter = mpi_root_rank_filter(tWorldCommunicator);

    auto tAttributeSet = boost::log::attribute_set{};
    tAttributeSet.insert(boost::log::attribute_name{kMPIRankAttributeName.data()},
                         boost::log::attributes::constant<int>(tWorldCommunicator.rank()));

    const auto tExpected = tWorldCommunicator.rank() == 0;
    EXPECT_EQ(tMPIRootRankFilter(boost::log::attribute_value_set{tAttributeSet, boost::log::attribute_set{},
                                                                 boost::log::attribute_set{}}),
              tExpected);
}
}  // namespace plato::third_party_integration::boost_log::parallel_unittest
