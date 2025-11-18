#ifndef PLATO_THIRD_PARTY_INTEGRATION_BOOST_LOG_MPIATTRIBUTES
#define PLATO_THIRD_PARTY_INTEGRATION_BOOST_LOG_MPIATTRIBUTES

#include <boost/log/expressions/filter.hpp>
#include <boost/log/expressions/keyword.hpp>
#include <boost/mpi/communicator.hpp>
#include <boost/mpi/environment.hpp>
#include <string_view>

#include "plato/third_party_integration/boost_log/AttributeTypes.hpp"

namespace plato::third_party_integration::boost_log
{
/// @brief Attribute for specifying the MPI rank of a log message.
struct MPIWorldCommRankAttribute
{
    using AttributeType = int;
    AttributeType mValue = boost::mpi::environment::initialized() ? boost::mpi::communicator{}.rank() : 0;

    [[nodiscard]] constexpr static inline auto name() -> std::string_view;

    /// @brief Returns a filter that filters out all MPI ranks except the root rank on @a aCommunicator.
    [[nodiscard]] static auto filter() -> boost::log::filter;
};

constexpr inline auto MPIWorldCommRankAttribute::name() -> std::string_view { return std::string_view{"MPI rank"}; }

static_assert(AttributeWithFilter<MPIWorldCommRankAttribute>,
              "MPIWorldCommRankAttribute satisfies concept AttributeWithFilter");

}  // namespace plato::third_party_integration::boost_log

BOOST_LOG_ATTRIBUTE_KEYWORD(
    mpi_rank_attribute,
    plato::third_party_integration::boost_log::MPIWorldCommRankAttribute::name().data(),
    typename plato::third_party_integration::boost_log::MPIWorldCommRankAttribute::AttributeType)

#endif
