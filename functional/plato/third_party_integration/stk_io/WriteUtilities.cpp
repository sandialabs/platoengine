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

template <stk::topology::rank_t Rank>
void write_mesh_scalar_field_impl(stk::io::StkMeshIoBroker& aIOBroker,
                                  const ScalarFieldFunction& aScalarField,
                                  const std::string_view aFieldName,
                                  const std::size_t aFileHandle)
{
    constexpr int tScalarFieldSize = 1;
    auto& tBulkData = aIOBroker.bulk_data();
    auto& tMetaData = tBulkData.mesh_meta_data();
    stk::mesh::Field<double>& tField = tMetaData.declare_field<double>(Rank, std::string{aFieldName}, tScalarFieldSize);
    constexpr double tInitialValue = 0;
    stk::mesh::put_field_on_mesh(tField, aIOBroker.meta_data().universal_part(), &tInitialValue);

    auto tEntityVector = std::vector<stk::mesh::Entity>{};
    stk::mesh::get_entities(aIOBroker.bulk_data(), Rank, tEntityVector, kUnsorted);
    for (const auto& tEntity : tEntityVector)
    {
        double* const tFieldData = stk::mesh::field_data(tField, tEntity);
        const auto tGlobalID = aIOBroker.bulk_data().identifier(tEntity);
        *tFieldData = aScalarField(tGlobalID);
    }

    aIOBroker.add_field(aFileHandle, tField);
}

void write_defined_output_fields(stk::io::StkMeshIoBroker& tIOBroker,
                                 const size_t aOutputFileIndex,
                                 const double aOutputTime)
{
    tIOBroker.begin_output_step(aOutputFileIndex, aOutputTime);
    tIOBroker.write_defined_output_fields(aOutputFileIndex);
    tIOBroker.end_output_step(aOutputFileIndex);
}

std::shared_ptr<stk::mesh::BulkData> bulk_data_from_description(const std::string_view aMeshDescription)
{
    std::shared_ptr<stk::mesh::BulkData> bulk = stk::mesh::MeshBuilder(MPI_COMM_SELF).create();
    bulk->mesh_meta_data().use_simple_fields();
    stk::io::fill_mesh(std::string{aMeshDescription}, *bulk);
    return bulk;
}

}  // namespace

auto create_io_mesh_broker(const std::filesystem::path& aInputMeshPath) -> std::unique_ptr<stk::io::StkMeshIoBroker>
{
    auto tIOBroker = std::make_unique<stk::io::StkMeshIoBroker>(MPI_COMM_SELF);
    tIOBroker->use_simple_fields();

    const size_t tIndex = tIOBroker->add_mesh_database(aInputMeshPath.string(), stk::io::READ_MESH);
    tIOBroker->set_active_mesh(tIndex);
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

void write_nodal_scalar_field(stk::io::StkMeshIoBroker& aIOBroker,
                              const ScalarFieldFunction& aScalarField,
                              const std::string_view aFieldName,
                              const std::size_t aFileHandle)
{
    write_mesh_scalar_field_impl<stk::topology::NODE_RANK>(aIOBroker, aScalarField, aFieldName, aFileHandle);
}

void write_element_scalar_field(stk::io::StkMeshIoBroker& aIOBroker,
                                const ScalarFieldFunction& aScalarField,
                                const std::string_view aFieldName,
                                const std::size_t aFileHandle)
{
    write_mesh_scalar_field_impl<stk::topology::ELEM_RANK>(aIOBroker, aScalarField, aFieldName, aFileHandle);
}

void finalize_mesh_data(std::unique_ptr<stk::io::StkMeshIoBroker>&& aIOBroker, const std::size_t aFileHandle)
{
    constexpr double tTime = 1.0;
    write_defined_output_fields(*aIOBroker, aFileHandle, tTime);
}

}  // namespace plato::third_party_integration::stk_io
