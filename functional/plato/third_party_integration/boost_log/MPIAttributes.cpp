#include "plato/third_party_integration/boost_log/MPIAttributes.hpp"

#include <boost/log/expressions.hpp>

namespace plato::third_party_integration::boost_log
{
auto MPIRankAttribute::filter() -> boost::log::filter
{
    constexpr auto tRootRank = 0;
    return boost::log::filter{boost::log::expressions::has_attr(mpi_rank_attribute) && mpi_rank_attribute == tRootRank};
}
}  // namespace plato::third_party_integration::boost_log
