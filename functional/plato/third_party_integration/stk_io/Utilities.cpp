#include "plato/third_party_integration/stk_io/Utilities.hpp"

#include <Ioss_DBUsage.h>  // for DatabaseUsage::READ_MODEL
#include <Ioss_ElementBlock.h>
#include <Ioss_Field.h>      // for Field, etc
#include <Ioss_IOFactory.h>  // for IOFactory
#include <Ioss_NodeBlock.h>  // for NodeBlock

#include <algorithm>
#include <boost/range.hpp>
#include <boost/range/adaptor/indexed.hpp>
#include <stk_io/FillMesh.hpp>
#include <stk_mesh/base/Field.hpp>
#include <stk_mesh/base/MeshBuilder.hpp>
#include <stk_mesh/base/MetaData.hpp>
#include <stk_mesh/base/Part.hpp>
#include <stk_search/Box.hpp>
#include <stk_topology/topology.hpp>
#include <stk_util/parallel/Parallel.hpp>

namespace plato::third_party_integration::stk_io
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

void write_mesh(const std::filesystem::path& aMeshName, const CommandGenerator& aCommandGenerator)
{
    write_bulk_data(aMeshName, generate_bulk_data(aCommandGenerator));
}

std::shared_ptr<stk::mesh::BulkData> generate_bulk_data(const CommandGenerator& aCommandGenerator)
{
    std::shared_ptr<stk::mesh::BulkData> bulk = stk::mesh::MeshBuilder(MPI_COMM_SELF).create();
    bulk->mesh_meta_data().use_simple_fields();
    stk::io::fill_mesh(aCommandGenerator.toString(), *bulk);
    return bulk;
}

void write_bulk_data(const std::filesystem::path& aMeshName, std::shared_ptr<stk::mesh::BulkData> aBulk)
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

std::vector<double> read_nodal_density(const std::filesystem::path& aMeshName)
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

unsigned int node_size(const stk::mesh::BulkData& aBulk) { return detail::size<stk::topology::NODE_RANK>(aBulk); }

unsigned int element_size(const stk::mesh::BulkData& aBulk) { return detail::size<stk::topology::ELEM_RANK>(aBulk); }

unsigned int block_size(const stk::mesh::BulkData& aBulk) { return aBulk.mesh_meta_data().get_mesh_parts().size(); }

std::optional<unsigned int> block_id(const stk::mesh::BulkData& aBulk, const std::string_view aBlockName)
{
    const auto& tParts = aBulk.mesh_meta_data().get_mesh_parts();
    const auto tPartWithName = std::find_if(tParts.cbegin(), tParts.cend(),
                                            [aBlockName](const auto& aPart) { return aBlockName == aPart->name(); });
    if (tPartWithName != tParts.cend())
    {
        return (*tPartWithName)->id();
    }
    else
    {
        return std::nullopt;
    }
}

unsigned int spatial_dimensions(const stk::mesh::BulkData& aBulk) { return aBulk.mesh_meta_data().spatial_dimension(); }

std::vector<double> flattened_nodal_coordinates(const stk::mesh::BulkData& aBulk)
{
    const unsigned int tSpatialDim = spatial_dimensions(aBulk);
    const auto tCoordinates = nodal_coordinates(aBulk);
    std::vector<double> tFlattenCoordinates(tCoordinates.size() * tSpatialDim, 0.0);

    for (auto const& tCoordinate : tCoordinates | boost::adaptors::indexed(0))
    {
        const auto tCoordinateVector = flatten(tCoordinate.value(), tSpatialDim);
        unsigned int tBaseIndex = static_cast<unsigned int>(tCoordinate.index() * tSpatialDim);
        std::copy(tCoordinateVector.begin(), tCoordinateVector.end(), tFlattenCoordinates.begin() + tBaseIndex);
    }

    return tFlattenCoordinates;
}

std::vector<common::Coordinate> nodal_coordinates(const stk::mesh::BulkData& aBulk)
{
    const unsigned int tSpatialDim = spatial_dimensions(aBulk);
    const unsigned int tNumberOfNodes = node_size(aBulk);
    std::vector<common::Coordinate> tCoordinates(static_cast<std::size_t>(tNumberOfNodes), {0.0, 0.0, 0.0});

    stk::mesh::EntityVector tNodeEntity;
    stk::mesh::get_entities(aBulk, stk::topology::NODE_RANK, tNodeEntity, true);
    const stk::mesh::FieldBase* const tCoordsField = aBulk.mesh_meta_data().coordinate_field();

    for (size_t tNodeIndex = 0; tNodeIndex < tNodeEntity.size(); tNodeIndex++)
    {
        const auto tData = static_cast<const double*>(stk::mesh::field_data(*tCoordsField, tNodeEntity[tNodeIndex]));
        tCoordinates[tNodeIndex].x = tData[0];
        tCoordinates[tNodeIndex].y = tData[1];
        tCoordinates[tNodeIndex].z = (tSpatialDim == 2 ? 0 : tData[2]);
    }
    return tCoordinates;
}

void write_nodal_density(const std::filesystem::path& aInputMeshName,
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

}  // namespace plato::third_party_integration::stk_io
