#include "plato/third_party_integration/boost_log/MPIAttributes.hpp"

#include <boost/log/expressions.hpp>

namespace plato::third_party_integration::boost_log
{
auto mpi_root_rank_filter(const boost::mpi::communicator& aCommunicator) -> boost::log::filter
{
    constexpr auto tRootRank = 0;
    return boost::log::filter{boost::log::expressions::has_attr(mpi_rank) && aCommunicator.rank() == tRootRank};
}
}  // namespace plato::third_party_integration::boost_log
