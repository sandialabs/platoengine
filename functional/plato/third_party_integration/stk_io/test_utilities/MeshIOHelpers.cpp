#include "plato/third_party_integration/stk_io/test_utilities/MeshIOHelpers.hpp"

#include <mpi.h>

#include <stk_io/FillMesh.hpp>
#include <stk_io/StkMeshIoBroker.hpp>
#include <stk_mesh/base/BulkData.hpp>
#include <stk_mesh/base/Field.hpp>
#include <stk_mesh/base/MeshBuilder.hpp>
#include <stk_mesh/base/MetaData.hpp>
#include <stk_topology/topology.hpp>

#include "plato/utilities/IndexRange.hpp"

namespace plato::third_party_integration::stk_io::test_utilities
{
namespace
{
constexpr auto kFieldTime = 1;

template <stk::topology::rank_t kRank>
auto read_field(const std::filesystem::path& aMeshName, std::string_view aFieldName) -> std::vector<double>
{
    const auto tIOBroker = std::make_shared<stk::io::StkMeshIoBroker>(MPI_COMM_SELF);
    tIOBroker->add_mesh_database(aMeshName.string(), stk::io::READ_MESH);
    tIOBroker->create_input_mesh();
    tIOBroker->populate_bulk_data();
    tIOBroker->meta_data().enable_late_fields();

    auto& tField = tIOBroker->meta_data().declare_field<double>(kRank, std::string{aFieldName});
    stk::mesh::put_field_on_entire_mesh(tField);
    tIOBroker->add_input_field(stk::io::MeshField{tField, std::string{aFieldName}});
    tIOBroker->read_defined_input_fields(kFieldTime);

    auto tEntities = stk::mesh::EntityVector{};
    constexpr auto tSorted = true;
    stk::mesh::get_entities(tIOBroker->bulk_data(), kRank, tEntities, tSorted);

    auto tFieldFromFile = std::vector<double>{};
    tFieldFromFile.reserve(tEntities.size());
    const auto tIndices = utilities::IndexRange{tEntities.size()};
    std::transform(tIndices.begin(), tIndices.end(), std::back_inserter(tFieldFromFile),
                   [&tField, &tEntities](const auto aEntityIndex)
                   { return static_cast<const double*>(stk::mesh::field_data(tField, tEntities[aEntityIndex]))[0]; });

    return tFieldFromFile;
}
}  // namespace

auto read_nodal_field(const std::filesystem::path& aMeshName, std::string_view aFieldName) -> std::vector<double>
{
    return read_field<stk::topology::NODE_RANK>(aMeshName, aFieldName);
}

auto read_element_field(const std::filesystem::path& aMeshName, std::string_view aFieldName) -> std::vector<double>
{
    return read_field<stk::topology::ELEMENT_RANK>(aMeshName, aFieldName);
}

}  // namespace plato::third_party_integration::stk_io::test_utilities