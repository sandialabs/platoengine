#include "plato/utilities/STKUtilities.hpp"

#include <Ioss_DBUsage.h>  // for DatabaseUsage::READ_MODEL
#include <Ioss_ElementBlock.h>
#include <Ioss_Field.h>      // for Field, etc
#include <Ioss_IOFactory.h>  // for IOFactory
#include <Ioss_NodeBlock.h>  // for NodeBlock

#include <stk_io/FillMesh.hpp>
#include <stk_mesh/base/Field.hpp>
#include <stk_mesh/base/MeshBuilder.hpp>
#include <stk_mesh/base/MetaData.hpp>
#include <stk_topology/topology.hpp>
#include <stk_util/parallel/Parallel.hpp>

namespace plato::utilities
{
namespace
{

std::shared_ptr<stk::io::StkMeshIoBroker> create_input_mesh_broker(const std::filesystem::path& aInputMeshName)
{
    std::shared_ptr<stk::io::StkMeshIoBroker> tIOBroker = std::make_shared<stk::io::StkMeshIoBroker>(MPI_COMM_SELF);
    tIOBroker->use_simple_fields();

    const size_t index = tIOBroker->add_mesh_database(aInputMeshName.string(), stk::io::READ_MESH);
    tIOBroker->set_active_mesh(index);
    tIOBroker->create_input_mesh();

    return tIOBroker;
}

void write_defined_output_fields(stk::io::StkMeshIoBroker& tIOBroker,
                                 const size_t aOutputFileIndex,
                                 const double aOutputTime)
{
    tIOBroker.begin_output_step(aOutputFileIndex, aOutputTime);
    tIOBroker.write_defined_output_fields(aOutputFileIndex);
    tIOBroker.end_output_step(aOutputFileIndex);
}
}  // namespace

std::shared_ptr<stk::mesh::BulkData> create_mesh(const std::string_view aGenerationCommand)
{
    std::shared_ptr<stk::mesh::BulkData> bulk = stk::mesh::MeshBuilder(MPI_COMM_SELF).create();
    bulk->mesh_meta_data().use_simple_fields();
    stk::io::fill_mesh(std::string{aGenerationCommand}, *bulk);
    return bulk;
}

void write_mesh(const std::filesystem::path& aMeshName, std::shared_ptr<stk::mesh::BulkData> aBulk)
{
    stk::io::StkMeshIoBroker tIOBroker;
    tIOBroker.set_bulk_data(std::move(aBulk));
    const size_t outputFileIndex = tIOBroker.create_output_mesh(aMeshName.string(), stk::io::WRITE_RESULTS);
    tIOBroker.write_output_mesh(outputFileIndex);
    tIOBroker.write_defined_output_fields(outputFileIndex);
}

std::shared_ptr<stk::mesh::BulkData> read_mesh_bulk_data(const std::filesystem::path& aMeshName)
{
    std::shared_ptr<stk::mesh::BulkData> tBulk = stk::mesh::MeshBuilder(MPI_COMM_SELF).create();
    stk::mesh::MetaData& tMeta = tBulk->mesh_meta_data();
    tMeta.use_simple_fields();
    stk::io::fill_mesh(aMeshName.string(), *tBulk);
    return tBulk;
}

std::vector<double> read_mesh_density(const std::filesystem::path& aMeshName)
{
    Ioss::DatabaseIO* tResultsDb =
        Ioss::IOFactory::create("exodus", aMeshName.string(), Ioss::READ_MODEL, MPI_COMM_SELF);
    Ioss::Region tResults(tResultsDb);

    tResults.begin_state(1);
    Ioss::NodeBlock* tNb = tResults.get_node_blocks()[0];
    std::vector<double> tNodeFieldData;
    tNb->get_field_data(std::string{detail::kTopologyFieldName}, tNodeFieldData);
    return tNodeFieldData;
}

std::vector<double> read_element_density(const std::filesystem::path& aMeshName)
{
    Ioss::DatabaseIO* tResultsDb =
        Ioss::IOFactory::create("exodus", aMeshName.string(), Ioss::READ_MODEL, MPI_COMM_SELF);
    Ioss::Region tResults(tResultsDb);

    tResults.begin_state(1);
    Ioss::ElementBlock* tEb = tResults.get_element_blocks()[0];
    std::vector<double> tElementFieldData;
    tEb->get_field_data(std::string{detail::kTopologyFieldName}, tElementFieldData);
    return tElementFieldData;
}

unsigned int element_size(const std::filesystem::path& aMeshName)
{
    const std::shared_ptr<stk::mesh::BulkData> tBulkData = read_mesh_bulk_data(aMeshName);
    assert(tBulkData);
    return element_size(*tBulkData);
}

unsigned int read_mesh_node_size(const std::filesystem::path& aMeshName)
{
    const std::shared_ptr<stk::mesh::BulkData> tBulkData = read_mesh_bulk_data(aMeshName);
    assert(tBulkData);
    return node_size(*tBulkData);
}

unsigned int node_size(const stk::mesh::BulkData& aBulk) { return detail::size<stk::topology::NODE_RANK>(aBulk); }

unsigned int element_size(const stk::mesh::BulkData& aBulk) { return detail::size<stk::topology::ELEM_RANK>(aBulk); }

unsigned int spatial_dimensions(const stk::mesh::BulkData& aBulk) { return aBulk.mesh_meta_data().spatial_dimension(); }

std::vector<double> nodal_coordinates(const stk::mesh::BulkData& aBulk)
{
    const unsigned int tSpatialDim = spatial_dimensions(aBulk);
    const unsigned int tNumberOfNodes = node_size(aBulk);
    std::vector<double> tCoordinates(static_cast<std::size_t>(tNumberOfNodes) * tSpatialDim, 0.0);

    stk::mesh::EntityVector tNodeEntity;
    stk::mesh::get_entities(aBulk, stk::topology::NODE_RANK, tNodeEntity, true);
    const stk::mesh::FieldBase* const tCoordsField = aBulk.mesh_meta_data().coordinate_field();

    for (size_t tNodeIndex = 0; tNodeIndex < tNodeEntity.size(); tNodeIndex++)
    {
        const auto tData = static_cast<const double*>(stk::mesh::field_data(*tCoordsField, tNodeEntity[tNodeIndex]));
        for (unsigned tCoordIndex = 0; tCoordIndex < tSpatialDim; tCoordIndex++)
        {
            tCoordinates[tNodeIndex * tSpatialDim + tCoordIndex] = tData[tCoordIndex];  // NOLINT
        }
    }
    return tCoordinates;
}

void write_mesh_density(const std::filesystem::path& aInputMeshName,
                        const std::vector<double>& aDensity,
                        const std::filesystem::path& aOutputMeshName)
{
    std::shared_ptr<stk::io::StkMeshIoBroker> tIOBroker =
        create_input_mesh_broker(aInputMeshName);  // todo : add communicator

    const size_t tOutputFileIndex =
        detail::write_mesh_density_impl<stk::topology::NODE_RANK>(*tIOBroker, aDensity, aOutputMeshName);

    constexpr double tTime = 1.0;
    write_defined_output_fields(*tIOBroker, tOutputFileIndex, tTime);
}

void write_element_density(const std::filesystem::path& aInputMeshName,
                           const std::vector<double>& aDensity,
                           const std::filesystem::path& aOutputMeshName)
{
    std::shared_ptr<stk::io::StkMeshIoBroker> tIOBroker =
        create_input_mesh_broker(aInputMeshName);  // todo : add communicator

    const size_t tOutputFileIndex =
        detail::write_mesh_density_impl<stk::topology::ELEMENT_RANK>(*tIOBroker, aDensity, aOutputMeshName);

    constexpr double tTime = 1.0;
    write_defined_output_fields(*tIOBroker, tOutputFileIndex, tTime);
}

stk::mesh::EntityVector element_vector(const stk::mesh::BulkData& aBulk)
{
    stk::mesh::EntityVector tElements;
    constexpr bool tSortById = true;
    stk::mesh::get_entities(aBulk, stk::topology::ELEM_RANK, aBulk.mesh_meta_data().locally_owned_part(), tElements,
                            tSortById);
    return tElements;
}

}  // namespace plato::utilities
