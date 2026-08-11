#include "plato/third_party_integration/krino/Utilities.hpp"

#include <algorithm>
#include <boost/mpi/communicator.hpp>
#include <cassert>
#include <iterator>
#include <stk_io/StkMeshIoBroker.hpp>  //get_selected_entities
#include <stk_mesh/base/GetEntities.hpp>
#include <stk_mesh/base/Types.hpp>
#include <stk_util/environment/EnvData.hpp>
#include <string>
#include <string_view>
#include <vector>

#include "plato/utilities/ContainerHelpers.hpp"

namespace plato::third_party_integration::krino
{
namespace
{
constexpr auto kLevelSetName = std::string_view{"LS"};
constexpr auto kVoidPhaseName = std::string_view{"void"};
}  // namespace

void set_stk_communicator_for_krino(const boost::mpi::communicator& aCommunicator)
{
    stk::EnvData::instance().initialize(aCommunicator);
}

auto retrieve_mpi_communicator_from_krino() -> boost::mpi::communicator
{
    return {stk::EnvData::instance().m_parallelComm, boost::mpi::comm_duplicate};
}

auto get_level_set_field_name() -> std::string { return std::string{kLevelSetName}; }

auto void_phase_name() -> std::string_view { return kVoidPhaseName; }

namespace detail
{
[[nodiscard]] auto get_ids_from_entities(const std::vector<stk::mesh::Entity>& aEntities,
                                         const stk::mesh::BulkData& aBulkData) -> std::vector<stk::mesh::EntityId>
{
    auto tIds = utilities::reserved_container<std::vector<stk::mesh::EntityId>>(aEntities.size());
    std::ranges::transform(aEntities, std::back_inserter(tIds),
                           [&aBulkData](const auto aEntity) { return aBulkData.entity_key(aEntity).id(); });

    std::ranges::sort(tIds);

    return tIds;
}
}  // namespace detail

}  // namespace plato::third_party_integration::krino
