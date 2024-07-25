#include "plato/third_party_integration/stk_io/Utilities.hpp"

#include <Ioss_DBUsage.h>  // for DatabaseUsage::READ_MODEL
#include <Ioss_ElementBlock.h>
#include <Ioss_Field.h>      // for Field, etc
#include <Ioss_IOFactory.h>  // for IOFactory
#include <Ioss_NodeBlock.h>  // for NodeBlock

#include <algorithm>
#include <boost/range.hpp>
#include <boost/range/adaptor/indexed.hpp>
#include <numeric>
#include <stk_io/FillMesh.hpp>
#include <stk_io/StkMeshIoBroker.hpp>
#include <stk_mesh/base/BulkData.hpp>
#include <stk_mesh/base/Field.hpp>
#include <stk_mesh/base/MeshBuilder.hpp>
#include <stk_mesh/base/MetaData.hpp>
#include <stk_topology/topology.hpp>
#include <stk_util/parallel/Parallel.hpp>

#include "plato/utilities/IndexRange.hpp"

namespace plato::third_party_integration::stk_io
{
namespace
{
constexpr auto kTopologyFieldName = std::string_view{"topology"};
constexpr bool kSortedByID = true;
constexpr bool kUnsorted = false;

template <stk::topology::rank_t Rank>
unsigned int size(const stk::mesh::BulkData& aBulk)
{
    std::vector<size_t> tEntityCounts;
    stk::mesh::comm_mesh_counts(aBulk, tEntityCounts);
    return tEntityCounts[Rank];
}

template <stk::topology::rank_t Rank>
size_t write_mesh_density_impl(stk::io::StkMeshIoBroker& aIOBroker,
                               const std::vector<double>& aDensity,
                               const std::filesystem::path& aOutputMeshName)
{
    constexpr int tScalarFieldSize = 1;
    stk::mesh::Field<double>& tField =
        aIOBroker.meta_data().declare_field<double>(Rank, std::string{kTopologyFieldName}, tScalarFieldSize);
    constexpr double tInitialValue = 0;
    stk::mesh::put_field_on_mesh(tField, aIOBroker.meta_data().universal_part(), &tInitialValue);
    aIOBroker.populate_bulk_data();

    std::vector<stk::mesh::Entity> tEntity;
    stk::mesh::get_entities(aIOBroker.bulk_data(), Rank, tEntity, kUnsorted);
    assert(tEntity.size() == aDensity.size());
    for (size_t iEntity = 0; iEntity < tEntity.size(); iEntity++)
    {
        double* const tFieldData = stk::mesh::field_data(tField, tEntity[iEntity]);
        *tFieldData = aDensity[aIOBroker.bulk_data().local_id(tEntity[iEntity])];
    }

    const size_t tOutputFileIndex = aIOBroker.create_output_mesh(aOutputMeshName.string(), stk::io::WRITE_RESULTS);
    aIOBroker.write_output_mesh(tOutputFileIndex);
    aIOBroker.add_field(tOutputFileIndex, tField);

    return tOutputFileIndex;
}

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

std::shared_ptr<stk::mesh::BulkData> bulk_data_from_description(const std::string_view aMeshDescription)
{
    std::shared_ptr<stk::mesh::BulkData> bulk = stk::mesh::MeshBuilder(MPI_COMM_SELF).create();
    bulk->mesh_meta_data().use_simple_fields();
    stk::io::fill_mesh(std::string{aMeshDescription}, *bulk);
    return bulk;
}

stk::mesh::Selector parts_to_selector(const PartReferenceVector& aParts)
{
    return std::accumulate(aParts.cbegin(), aParts.cend(), stk::mesh::Selector{},
                           [](stk::mesh::Selector aSelector, const auto tPart)
                           {
                               aSelector |= tPart.get();
                               return aSelector;
                           });
}

PartReferenceVector universal_part(const stk::mesh::BulkData& aBulk)
{
    return PartReferenceVector{std::cref(aBulk.mesh_meta_data().universal_part())};
}

}  // namespace

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
    tNb->get_field_data(std::string{kTopologyFieldName}, tNodeFieldData);
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
    tEb->get_field_data(std::string{kTopologyFieldName}, tElementFieldData);
    return tElementFieldData;
}

unsigned int node_size(const stk::mesh::BulkData& aBulk) { return size<stk::topology::NODE_RANK>(aBulk); }

unsigned int element_size(const stk::mesh::BulkData& aBulk) { return size<stk::topology::ELEM_RANK>(aBulk); }

unsigned int spatial_dimensions(const stk::mesh::BulkData& aBulk) { return aBulk.mesh_meta_data().spatial_dimension(); }

std::vector<double> flattened_nodal_coordinates(const stk::mesh::BulkData& aBulk)
{
    const unsigned int tSpatialDim = spatial_dimensions(aBulk);
    const auto tCoordinates = nodal_coordinates(aBulk);
    std::vector<double> tFlattenCoordinates(tCoordinates.size() * tSpatialDim);

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
    return nodal_coordinates(aBulk, universal_part(aBulk));
}

auto nodal_coordinates(const stk::mesh::BulkData& aBulk, const PartReferenceVector& aParts)
    -> std::vector<common::Coordinate>
{
    auto tNodeEntity = stk::mesh::EntityVector{};
    stk::mesh::get_entities(aBulk, stk::topology::NODE_RANK, parts_to_selector(aParts), tNodeEntity, kSortedByID);

    const stk::mesh::FieldBase* const tCoordsField = aBulk.mesh_meta_data().coordinate_field();

    auto tCoordinates = std::vector<common::Coordinate>{};
    tCoordinates.reserve(tNodeEntity.size());

    const auto tIndices = utilities::IndexRange{tNodeEntity.size()};
    std::transform(tIndices.begin(), tIndices.end(), std::back_inserter(tCoordinates),
                   [tSpatialDim = spatial_dimensions(aBulk), tCoordsField, &tNodeEntity](const auto aNodeIndex)
                   {
                       const auto tData =
                           static_cast<const double*>(stk::mesh::field_data(*tCoordsField, tNodeEntity[aNodeIndex]));
                       return common::Coordinate{tData[0], tData[1], tSpatialDim == 2 ? 0 : tData[2]};
                   });
    return tCoordinates;
}

void write_nodal_density(const std::filesystem::path& aInputMeshName,
                         const std::vector<double>& aDensity,
                         const std::filesystem::path& aOutputMeshName)
{
    std::shared_ptr<stk::io::StkMeshIoBroker> tIOBroker =
        create_input_mesh_broker(aInputMeshName);  // todo : add communicator

    const size_t tOutputFileIndex =
        write_mesh_density_impl<stk::topology::NODE_RANK>(*tIOBroker, aDensity, aOutputMeshName);

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
        write_mesh_density_impl<stk::topology::ELEMENT_RANK>(*tIOBroker, aDensity, aOutputMeshName);

    constexpr double tTime = 1.0;
    write_defined_output_fields(*tIOBroker, tOutputFileIndex, tTime);
}

stk::mesh::EntityVector element_vector(const stk::mesh::BulkData& aBulk)
{
    return element_vector(aBulk, aBulk.mesh_meta_data().universal_part());
}

stk::mesh::EntityVector element_vector(const stk::mesh::BulkData& aBulk, const stk::mesh::Part& aPart)
{
    return element_vector(aBulk, PartReferenceVector{std::cref(aPart)});
}

stk::mesh::EntityVector element_vector(const stk::mesh::BulkData& aBulk, const PartReferenceVector& aParts)
{
    stk::mesh::EntityVector tElements;
    stk::mesh::get_entities(aBulk, stk::topology::ELEM_RANK, parts_to_selector(aParts), tElements, kSortedByID);
    return tElements;
}

}  // namespace plato::third_party_integration::stk_io
