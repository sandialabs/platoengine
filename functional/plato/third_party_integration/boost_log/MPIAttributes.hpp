#ifndef PLATO_THIRD_PARTY_INTEGRATION_BOOST_LOG_MPIATTRIBUTES
#define PLATO_THIRD_PARTY_INTEGRATION_BOOST_LOG_MPIATTRIBUTES

#include <boost/log/expressions/filter.hpp>
#include <boost/log/expressions/keyword.hpp>
#include <string_view>

namespace plato::third_party_integration::boost_log
{
/// @brief Returns a filter that filters out all MPI ranks except the root rank on @a aCommunicator.
[[nodiscard]] auto mpi_root_rank_filter() -> boost::log::filter;

constexpr inline auto kMPIRankAttributeName = std::string_view{"MPI rank"};
}  // namespace plato::third_party_integration::boost_log

BOOST_LOG_ATTRIBUTE_KEYWORD(mpi_rank_attribute,
                            plato::third_party_integration::boost_log::kMPIRankAttributeName.data(),
                            int)

#endif
