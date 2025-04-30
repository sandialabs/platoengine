#include "plato/third_party_integration/stk_io/ReadUtilities.hpp"

#include <algorithm>
#include <iterator>
#include <numeric>
#include <stk_io/FillMesh.hpp>
#include <stk_io/StkMeshIoBroker.hpp>
#include <stk_mesh/base/BulkData.hpp>
#include <stk_mesh/base/Comm.hpp>
#include <stk_mesh/base/Entity.hpp>
#include <stk_mesh/base/Field.hpp>
#include <stk_mesh/base/GetEntities.hpp>
#include <stk_mesh/base/MeshBuilder.hpp>
#include <stk_mesh/base/MetaData.hpp>
#include <stk_topology/topology.hpp>
#include <stk_util/parallel/Parallel.hpp>

#include "plato/third_party_integration/stk_io/BlockUtilities.hpp"
#include "plato/utilities/IndexRange.hpp"

namespace plato::third_party_integration::stk_io
{
namespace
{
constexpr bool kSortedByID = true;

stk::mesh::Selector parts_to_selector(const PartReferenceVector& aParts)
{
    return std::accumulate(aParts.cbegin(), aParts.cend(), stk::mesh::Selector{},
                           [](stk::mesh::Selector aSelector, const auto tPart)
                           {
                               aSelector |= tPart.get();
                               return aSelector;
                           });
}

template <stk::topology::rank_t Rank>
unsigned int size(const stk::mesh::BulkData& aBulk, const PartReferenceVector& aParts)
{
    std::vector<size_t> tEntityCounts;
    const auto tSelector = parts_to_selector(aParts);
    stk::mesh::comm_mesh_counts(aBulk, tEntityCounts, &tSelector);
    return tEntityCounts[Rank];
}

PartReferenceVector universal_part(const stk::mesh::BulkData& aBulk)
{
    return PartReferenceVector{std::cref(aBulk.mesh_meta_data().universal_part())};
}

[[nodiscard]] auto create_reading_iobroker(const std::filesystem::path& aInputMeshName)
    -> std::unique_ptr<stk::io::StkMeshIoBroker>
{
    auto tIOBroker = std::make_unique<stk::io::StkMeshIoBroker>(MPI_COMM_SELF);
    tIOBroker->add_mesh_database(aInputMeshName.string(), stk::io::READ_MESH);
    tIOBroker->create_input_mesh();
    tIOBroker->populate_bulk_data();
    tIOBroker->meta_data().enable_late_fields();
    tIOBroker->add_all_mesh_fields_as_input_fields();
    return tIOBroker;
}

template <stk::topology::rank_t Rank>
[[nodiscard]] bool field_exists(const stk::io::StkMeshIoBroker& aIOBroker, const std::string_view aFieldName)
{
    const auto tField = aIOBroker.meta_data().get_field(Rank, std::string{aFieldName});
    return tField;
}

/// @pre `aIOBroker.get_time_steps().empty()` must be `false`.
[[nodiscard]] auto time_step(const stk::io::StkMeshIoBroker& aIOBroker, const TimeStep aTimeStep) -> double
{
    return aTimeStep.valueOrInvoke(
        [&aIOBroker]()
        {
            const auto tTimeSteps = aIOBroker.get_time_steps();
            const auto tMaxElementIter = std::max_element(tTimeSteps.begin(), tTimeSteps.end());
            assert(tMaxElementIter != tTimeSteps.end());
            return *tMaxElementIter;
        });
}

[[nodiscard]] bool time_step_exists(const stk::io::StkMeshIoBroker& aIOBroker, const TimeStep aTimeStep)
{
    if (aIOBroker.get_time_steps().empty())
    {
        return false;
    }
    const auto tTimeStep = time_step(aIOBroker, aTimeStep);
    const auto tTimeSteps = aIOBroker.get_time_steps();
    return std::find(tTimeSteps.begin(), tTimeSteps.end(), tTimeStep) != tTimeSteps.end();
}

template <stk::topology::rank_t Rank>
[[nodiscard]] auto get_all_field_names(const std::filesystem::path& aInputMeshName) -> std::vector<std::string>
{
    const auto tIOBroker = create_reading_iobroker(aInputMeshName);
    const auto tFieldVector = tIOBroker->meta_data().get_fields(Rank);
    std::vector<std::string> tFieldNames;
    tFieldNames.reserve(tFieldVector.size());
    std::transform(tFieldVector.begin(), tFieldVector.end(), std::back_inserter(tFieldNames),
                   [](const auto& aField) { return aField->name(); });
    return tFieldNames;
}

template <stk::topology::rank_t Rank>
[[nodiscard]] auto read_field(const stk::io::StkMeshIoBroker& aIOBroker, const std::string_view aFieldName)
    -> std::map<std::size_t, double>
{
    auto tField = aIOBroker.meta_data().get_field(Rank, std::string{aFieldName});

    auto tEntities = stk::mesh::EntityVector{};
    constexpr auto tSorted = true;
    stk::mesh::get_entities(aIOBroker.bulk_data(), Rank, tEntities, tSorted);

    auto tFieldFromFile = std::map<std::size_t, double>{};
    const auto tIndices = utilities::IndexRange{tEntities.size()};
    std::transform(tIndices.begin(), tIndices.end(), std::inserter(tFieldFromFile, tFieldFromFile.end()),
                   [tField, &tEntities, &tBulk = aIOBroker.bulk_data()](const auto aEntityIndex)
                   {
                       const auto tGlobalID = tBulk.entity_key(tEntities[aEntityIndex]).id();
                       return std::make_pair(tGlobalID, static_cast<const double*>(stk::mesh::field_data(
                                                            *tField, tEntities[aEntityIndex]))[0]);
                   });

    return tFieldFromFile;
}

auto entity_ids(const stk::mesh::BulkData& aBulk,
                const PartReferenceVector& aParts,
                const stk::topology::rank_t aEntityType) -> std::vector<std::size_t>
{
    auto tEntityVector = stk::mesh::EntityVector{};
    stk::mesh::get_entities(aBulk, aEntityType, parts_to_selector(aParts), tEntityVector, kSortedByID);

    auto tIDs = std::vector<std::size_t>{};
    tIDs.reserve(tEntityVector.size());

    std::transform(tEntityVector.begin(), tEntityVector.end(), std::back_inserter(tIDs),
                   [&aBulk](const auto aEntity) { return aBulk.entity_key(aEntity).id(); });
    return tIDs;
}

}  // namespace

std::shared_ptr<stk::mesh::BulkData> read_mesh_bulk_data(const std::filesystem::path& aMeshName)
{
    std::shared_ptr<stk::mesh::BulkData> tBulk = stk::mesh::MeshBuilder(MPI_COMM_SELF).create();
    stk::mesh::MetaData& tMeta = tBulk->mesh_meta_data();
    tMeta.use_simple_fields();
    stk::io::fill_mesh(aMeshName.string(), *tBulk);
    return tBulk;
}

unsigned int node_size(const stk::mesh::BulkData& aBulk) { return node_size(aBulk, universal_part(aBulk)); }

unsigned int node_size(const stk::mesh::BulkData& aBulk, const PartReferenceVector& aParts)
{
    return size<stk::topology::NODE_RANK>(aBulk, aParts);
}

unsigned int element_size(const stk::mesh::BulkData& aBulk) { return element_size(aBulk, universal_part(aBulk)); }

unsigned int element_size(const stk::mesh::BulkData& aBulk, const PartReferenceVector& aParts)
{
    return size<stk::topology::ELEM_RANK>(aBulk, aParts);
}

unsigned int spatial_dimensions(const stk::mesh::BulkData& aBulk) { return aBulk.mesh_meta_data().spatial_dimension(); }

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

auto node_ids(const stk::mesh::BulkData& aBulk, const PartReferenceVector& aParts) -> std::vector<std::size_t>
{
    return entity_ids(aBulk, aParts, stk::topology::NODE_RANK);
}

auto node_ids(const stk::mesh::BulkData& aBulk, const stk::mesh::Part& aPart) -> std::vector<std::size_t>
{
    return entity_ids(aBulk, {std::cref(aPart)}, stk::topology::NODE_RANK);
}

auto element_ids(const stk::mesh::BulkData& aBulk, const stk::mesh::Part& aPart) -> std::vector<std::size_t>
{
    return entity_ids(aBulk, {std::cref(aPart)}, stk::topology::ELEMENT_RANK);
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

bool element_field_exists(const std::filesystem::path& aInputMeshName, const std::string_view aFieldName)
{
    const auto tIOBroker = create_reading_iobroker(aInputMeshName);
    return field_exists<stk::topology::ELEMENT_RANK>(*tIOBroker, aFieldName);
}

bool nodal_field_exists(const std::filesystem::path& aInputMeshName, const std::string_view aFieldName)
{
    const auto tIOBroker = create_reading_iobroker(aInputMeshName);
    return field_exists<stk::topology::NODE_RANK>(*tIOBroker, aFieldName);
}

auto nodal_field_names(const std::filesystem::path& aInputMeshName) -> std::vector<std::string>
{
    return get_all_field_names<stk::topology::NODE_RANK>(aInputMeshName);
}

auto read_element_field(const std::filesystem::path& aInputMeshName,
                        const std::string_view aFieldName,
                        const TimeStep aTime) -> std::map<std::size_t, double>
{
    const auto tIOBroker = create_reading_iobroker(aInputMeshName);
    if (field_exists<stk::topology::ELEMENT_RANK>(*tIOBroker, aFieldName) && time_step_exists(*tIOBroker, aTime))
    {
        tIOBroker->read_defined_input_fields(time_step(*tIOBroker, aTime));
        return read_field<stk::topology::ELEMENT_RANK>(*tIOBroker, aFieldName);
    }
    return {};
}

auto read_nodal_field(const std::filesystem::path& aInputMeshName,
                      const std::string_view aFieldName,
                      const TimeStep aTime) -> std::map<std::size_t, double>
{
    const auto tIOBroker = create_reading_iobroker(aInputMeshName);
    if (field_exists<stk::topology::NODE_RANK>(*tIOBroker, aFieldName) && time_step_exists(*tIOBroker, aTime))
    {
        tIOBroker->read_defined_input_fields(time_step(*tIOBroker, aTime));
        return read_field<stk::topology::NODE_RANK>(*tIOBroker, aFieldName);
    }
    return {};
}

auto time_steps(const std::filesystem::path& aInputMeshName) -> std::vector<double>
{
    return create_reading_iobroker(aInputMeshName)->get_time_steps();
}
}  // namespace plato::third_party_integration::stk_io
