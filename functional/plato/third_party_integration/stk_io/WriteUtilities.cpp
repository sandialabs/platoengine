#include "plato/third_party_integration/stk_io/WriteUtilities.hpp"

#include <Ioss_NodeBlock.h>

#include <stk_io/FillMesh.hpp>
#include <stk_io/StkMeshIoBroker.hpp>
#include <stk_mesh/base/BulkData.hpp>
#include <stk_mesh/base/Field.hpp>
#include <stk_mesh/base/GetEntities.hpp>
#include <stk_mesh/base/MeshBuilder.hpp>
#include <stk_mesh/base/MetaData.hpp>
#include <stk_topology/topology.hpp>
#include <stk_util/parallel/Parallel.hpp>

namespace plato::third_party_integration::stk_io
{
namespace
{
constexpr bool kUnsorted = false;

std::shared_ptr<stk::mesh::BulkData> bulk_data_from_description(const std::string_view aMeshDescription)
{
    std::shared_ptr<stk::mesh::BulkData> bulk = stk::mesh::MeshBuilder(MPI_COMM_SELF).create();
    bulk->mesh_meta_data().use_simple_fields();
    stk::io::fill_mesh(std::string{aMeshDescription}, *bulk);
    return bulk;
}

template <stk::topology::rank_t Rank>
void initialize_scalar_output_field_impl(stk::mesh::MetaData& aMetaData, const std::string_view aFieldName)
{
    constexpr int tScalarFieldSize = 1;
    stk::mesh::Field<double>& tField = aMetaData.declare_field<double>(Rank, std::string{aFieldName}, tScalarFieldSize);
    constexpr double tInitialValue = 0;
    stk::mesh::put_field_on_mesh(tField, aMetaData.universal_part(), &tInitialValue);
}

template <stk::topology::rank_t Rank>
void populate_scalar_field_values_impl(stk::io::StkMeshIoBroker& aIOBroker,
                                       const std::string_view aFieldName,
                                       const ScalarFieldFunction& aScalarField)
{
    auto tEntityVector = std::vector<stk::mesh::Entity>{};
    stk::mesh::get_entities(aIOBroker.bulk_data(), Rank, tEntityVector, kUnsorted);

    const stk::mesh::Field<double>* tField = aIOBroker.meta_data().get_field<double>(Rank, std::string{aFieldName});

    for (const auto& tEntity : tEntityVector)
    {
        double* const tFieldData = stk::mesh::field_data(*tField, tEntity);
        const auto tGlobalID = aIOBroker.bulk_data().identifier(tEntity);
        *tFieldData = aScalarField(tGlobalID);
    }
}

template <stk::topology::rank_t Rank>
void add_field_to_output_file_impl(stk::io::StkMeshIoBroker& aIOBroker,
                                   const size_t aFileHandle,
                                   const std::string_view aFieldName)

{
    stk::mesh::Field<double>* tField = aIOBroker.meta_data().get_field<double>(Rank, std::string{aFieldName});
    aIOBroker.add_field(aFileHandle, *tField);
}
}  // namespace

auto create_io_broker_from_input_file(const std::filesystem::path& aInputMeshPath)
    -> std::unique_ptr<stk::io::StkMeshIoBroker>
{
    auto tIOBroker = std::make_unique<stk::io::StkMeshIoBroker>(MPI_COMM_SELF);
    tIOBroker->use_simple_fields();

    tIOBroker->add_mesh_database(aInputMeshPath.string(), stk::io::READ_MESH);
    tIOBroker->create_input_mesh();
    tIOBroker->populate_bulk_data();
    tIOBroker->meta_data().enable_late_fields();

    return tIOBroker;
}

auto create_output_mesh(const std::filesystem::path& aOutputMeshPath, stk::io::StkMeshIoBroker& aIOBroker)
    -> std::size_t
{
    return aIOBroker.create_output_mesh(aOutputMeshPath.string(), stk::io::WRITE_RESULTS);
}

void write_mesh(const std::filesystem::path& aMeshName, const CommandGenerator& aCommandGenerator)
{
    write_bulk_data(aMeshName, generate_bulk_data(aCommandGenerator));
}

void write_mesh(const std::filesystem::path& aMeshName, std::string_view aMeshDescription)
{
    write_bulk_data(aMeshName, bulk_data_from_description(aMeshDescription));
}

std::shared_ptr<stk::mesh::BulkData> generate_bulk_data(const CommandGenerator& aCommandGenerator)
{
    return bulk_data_from_description(aCommandGenerator.toString());
}

void write_bulk_data(const std::filesystem::path& aMeshName, std::shared_ptr<stk::mesh::BulkData> aBulk)
{
    stk::io::StkMeshIoBroker tIOBroker;
    tIOBroker.set_bulk_data(std::move(aBulk));
    const size_t outputFileIndex = tIOBroker.create_output_mesh(aMeshName.string(), stk::io::WRITE_RESULTS);
    tIOBroker.write_output_mesh(outputFileIndex);
    tIOBroker.write_defined_output_fields(outputFileIndex);
}

auto create_io_broker_from_bulk(stk::mesh::BulkData& aBulk) -> std::unique_ptr<stk::io::StkMeshIoBroker>
{
    auto tIOBroker = std::make_unique<stk::io::StkMeshIoBroker>(MPI_COMM_SELF);
    tIOBroker->set_bulk_data(aBulk);
    tIOBroker->meta_data().enable_late_fields();

    return tIOBroker;
}

void initialize_element_scalar_field(stk::io::StkMeshIoBroker& aIOBroker, const std::string_view aFieldName)
{
    initialize_scalar_output_field_impl<stk::topology::ELEM_RANK>(aIOBroker.meta_data(), aFieldName);
}

void initialize_nodal_scalar_field(stk::io::StkMeshIoBroker& aIOBroker, const std::string_view aFieldName)
{
    initialize_scalar_output_field_impl<stk::topology::NODE_RANK>(aIOBroker.meta_data(), aFieldName);
}

void populate_element_scalar_field_values(stk::io::StkMeshIoBroker& aIOBroker,
                                          const std::string_view aFieldName,
                                          const ScalarFieldFunction& aScalarField)
{
    populate_scalar_field_values_impl<stk::topology::ELEM_RANK>(aIOBroker, aFieldName, aScalarField);
}

void populate_nodal_scalar_field_values(stk::io::StkMeshIoBroker& aIOBroker,
                                        const std::string_view aFieldName,
                                        const ScalarFieldFunction& aScalarField)
{
    populate_scalar_field_values_impl<stk::topology::NODE_RANK>(aIOBroker, aFieldName, aScalarField);
}

void add_nodal_field_to_output_file(stk::io::StkMeshIoBroker& aIOBroker,
                                    const size_t aFileHandle,
                                    const std::string_view aFieldName)
{
    add_field_to_output_file_impl<stk::topology::NODE_RANK>(aIOBroker, aFileHandle, aFieldName);
}

void add_element_field_to_output_file(stk::io::StkMeshIoBroker& aIOBroker,
                                      const size_t aFileHandle,
                                      const std::string_view aFieldName)
{
    add_field_to_output_file_impl<stk::topology::ELEM_RANK>(aIOBroker, aFileHandle, aFieldName);
}

void write_fields_at_time(stk::io::StkMeshIoBroker& aIOBroker, const size_t aFileHandle, double aOutputTime)
{
    aIOBroker.process_output_request(aFileHandle, aOutputTime);
}

}  // namespace plato::third_party_integration::stk_io
