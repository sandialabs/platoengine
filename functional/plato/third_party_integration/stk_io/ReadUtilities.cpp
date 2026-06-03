#include "plato/third_party_integration/stk_io/ReadUtilities.hpp"

#include <mpi.h>

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <filesystem>
#include <functional>
#include <iterator>
#include <map>
#include <memory>
#include <numeric>
#include <ranges>
#include <span>
#include <stk_io/DatabasePurpose.hpp>
#include <stk_io/FillMesh.hpp>
#include <stk_io/StkMeshIoBroker.hpp>
#include <stk_mesh/base/BulkData.hpp>
#include <stk_mesh/base/Comm.hpp>
#include <stk_mesh/base/FieldBase.hpp>
#include <stk_mesh/base/GetEntities.hpp>
#include <stk_mesh/base/MetaData.hpp>
#include <stk_topology/topology_decl.hpp>
#include <string_view>
#include <utility>
#include <vector>

#include "plato/third_party_integration/common/BoundingBox.hpp"
#include "plato/third_party_integration/common/Vector3.hpp"
#include "plato/third_party_integration/stk_io/BlockUtilities.hpp"
#include "plato/utilities/IndexRange.hpp"

namespace plato::third_party_integration::stk_io
{
namespace
{

constexpr bool kSortedByID = true;

template <std::size_t kFieldSize>
[[nodiscard]] auto to_span(void* const aStkField)
{
    return std::span<double, kFieldSize>(static_cast<double*>(aStkField), kFieldSize);
}

template <stk::topology::rank_t kRank>
auto size(const stk::mesh::BulkData& aBulk, const PartReferenceVector& aParts) -> unsigned int
{
    std::vector<size_t> tEntityCounts;
    const auto tSelector = detail::parts_to_selector(aParts);
    stk::mesh::comm_mesh_counts(aBulk, tEntityCounts, &tSelector);
    return tEntityCounts[kRank];
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

template <stk::topology::rank_t kRank>
[[nodiscard]] auto field_exists(const stk::io::StkMeshIoBroker& aIOBroker, const std::string_view aFieldName) -> bool
{
    auto* const tField = aIOBroker.meta_data().get_field(kRank, std::string{aFieldName});
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

[[nodiscard]] auto time_step_exists(const stk::io::StkMeshIoBroker& aIOBroker, const TimeStep aTimeStep) -> bool
{
    if (aIOBroker.get_time_steps().empty())
    {
        return false;
    }
    const auto tTimeStep = time_step(aIOBroker, aTimeStep);
    const auto tTimeSteps = aIOBroker.get_time_steps();
    return std::find(tTimeSteps.begin(), tTimeSteps.end(), tTimeStep) != tTimeSteps.end();
}

template <stk::topology::rank_t kRank>
[[nodiscard]] auto get_all_field_names(const std::filesystem::path& aInputMeshName) -> std::vector<std::string>
{
    const auto tIOBroker = create_reading_iobroker(aInputMeshName);
    const auto tFieldVector = tIOBroker->meta_data().get_fields(kRank);
    std::vector<std::string> tFieldNames;
    tFieldNames.reserve(tFieldVector.size());
    std::transform(tFieldVector.begin(), tFieldVector.end(), std::back_inserter(tFieldNames),
                   [](const auto& aField) { return aField->name(); });
    return tFieldNames;
}

template <stk::topology::rank_t kRank>
[[nodiscard]] auto read_field(const stk::io::StkMeshIoBroker& aIOBroker,
                              const std::string_view aFieldName) -> std::map<std::size_t, double>
{
    auto* tField = aIOBroker.meta_data().get_field(kRank, std::string{aFieldName});

    auto tEntities = stk::mesh::EntityVector{};
    constexpr auto tSorted = true;
    stk::mesh::get_entities(aIOBroker.bulk_data(), kRank, tEntities, tSorted);

    auto tFieldFromFile = std::map<std::size_t, double>{};
    const auto tIndices = utilities::IndexRange{tEntities.size()};
    std::transform(tIndices.begin(), tIndices.end(), std::inserter(tFieldFromFile, tFieldFromFile.end()),
                   [tField, &tEntities, &tBulk = aIOBroker.bulk_data()](const auto aEntityIndex)
                   {
                       const auto tGlobalID = tBulk.entity_key(tEntities[aEntityIndex]).id();
                       constexpr auto tFieldSize = std::size_t{1U};
                       const auto tFieldData =
                           to_span<tFieldSize>(stk::mesh::field_data(*tField, tEntities[aEntityIndex]));
                       return std::make_pair(tGlobalID, tFieldData[0]);
                   });

    return tFieldFromFile;
}

auto entity_ids(const stk::mesh::BulkData& aBulk,
                const PartReferenceVector& aParts,
                const stk::topology::rank_t aEntityType) -> std::vector<std::size_t>
{
    auto tEntityVector = stk::mesh::EntityVector{};
    stk::mesh::get_entities(aBulk, aEntityType, detail::parts_to_selector(aParts), tEntityVector, kSortedByID);

    auto tIDs = std::vector<std::size_t>{};
    tIDs.reserve(tEntityVector.size());

    std::transform(tEntityVector.begin(), tEntityVector.end(), std::back_inserter(tIDs),
                   [&aBulk](const auto aEntity) { return aBulk.entity_key(aEntity).id(); });
    return tIDs;
}

}  // namespace

auto read_mesh_bulk_data(const std::filesystem::path& aMeshName) -> std::shared_ptr<stk::mesh::BulkData>
{
    return create_reading_iobroker(aMeshName)->bulk_data_ptr();
}

auto node_size(const stk::mesh::BulkData& aBulk) -> unsigned int { return node_size(aBulk, universal_part(aBulk)); }

auto node_size(const stk::mesh::BulkData& aBulk, const PartReferenceVector& aParts) -> unsigned int
{
    return size<stk::topology::NODE_RANK>(aBulk, aParts);
}

auto element_size(const stk::mesh::BulkData& aBulk) -> unsigned int
{
    return element_size(aBulk, universal_part(aBulk));
}

auto element_size(const stk::mesh::BulkData& aBulk, const PartReferenceVector& aParts) -> unsigned int
{
    return size<stk::topology::ELEM_RANK>(aBulk, aParts);
}

auto spatial_dimensions(const stk::mesh::BulkData& aBulk) -> unsigned int
{
    return aBulk.mesh_meta_data().spatial_dimension();
}

auto nodal_coordinates(const stk::mesh::BulkData& aBulk) -> std::vector<common::Coordinate>
{
    return nodal_coordinates(aBulk, universal_part(aBulk));
}

namespace
{
[[nodiscard]] auto stk_nodal_coordinate_view(const stk::mesh::BulkData& aBulk, const PartReferenceVector& aParts)
{
    auto tNodeEntity = stk::mesh::EntityVector{};
    stk::mesh::get_entities(aBulk, stk::topology::NODE_RANK, detail::parts_to_selector(aParts), tNodeEntity,
                            kSortedByID);
    const auto& tCoordinateField = *aBulk.mesh_meta_data().coordinate_field();

    const auto tIndices = std::views::iota(0U, tNodeEntity.size());
    return tIndices |
           std::views::transform(
               [tSpatialDim = spatial_dimensions(aBulk), &tCoordinateField,
                tNodeEntity = std::move(tNodeEntity)](const auto aNodeIndex)
               {
                   constexpr auto tCoordinateFieldSize = std::size_t{3U};
                   const auto tData =
                       to_span<tCoordinateFieldSize>(stk::mesh::field_data(tCoordinateField, tNodeEntity[aNodeIndex]));
                   return common::Coordinate{tData[0], tData[1], tSpatialDim == 2 ? 0 : tData[2]};
               }) |
           std::views::common;
}

}  // namespace

auto nodal_coordinates(const stk::mesh::BulkData& aBulk, const PartReferenceVector& aParts)
    -> std::vector<common::Coordinate>
{
    const auto tCoordinates = stk_nodal_coordinate_view(aBulk, aParts);
    return {tCoordinates.begin(), tCoordinates.end()};
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

auto element_vector(const stk::mesh::BulkData& aBulk) -> stk::mesh::EntityVector
{
    return element_vector(aBulk, aBulk.mesh_meta_data().universal_part());
}

auto element_vector(const stk::mesh::BulkData& aBulk, const stk::mesh::Part& aPart) -> stk::mesh::EntityVector
{
    return element_vector(aBulk, PartReferenceVector{std::cref(aPart)});
}

auto element_vector(const stk::mesh::BulkData& aBulk, const PartReferenceVector& aParts) -> stk::mesh::EntityVector
{
    stk::mesh::EntityVector tElements;
    stk::mesh::get_entities(aBulk, stk::topology::ELEM_RANK, detail::parts_to_selector(aParts), tElements, kSortedByID);
    return tElements;
}

auto element_field_exists(const std::filesystem::path& aInputMeshName, const std::string_view aFieldName) -> bool
{
    const auto tIOBroker = create_reading_iobroker(aInputMeshName);
    return field_exists<stk::topology::ELEMENT_RANK>(*tIOBroker, aFieldName);
}

auto nodal_field_exists(const std::filesystem::path& aInputMeshName, const std::string_view aFieldName) -> bool
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

auto bounding_box(const stk::mesh::BulkData& aBulkData, const PartReferenceVector& aParts) -> common::BoundingBox
{
    const auto tCoordinates = stk_nodal_coordinate_view(aBulkData, aParts);
    return common::bounding_box(tCoordinates);
}

auto universal_part(const stk::mesh::BulkData& aBulk) -> PartReferenceVector
{
    return PartReferenceVector{std::cref(aBulk.mesh_meta_data().universal_part())};
}

auto all_tet_or_tri_elements(const stk::mesh::BulkData& aBulk) -> bool
{
    const auto tParts = aBulk.mesh_meta_data().get_mesh_parts();
    return std::all_of(tParts.begin(), tParts.end(), [](const auto& aPart) { return all_tet_or_tri_elements(*aPart); });
}

auto all_tet_or_tri_elements(const stk::mesh::Part& aPart) -> bool
{
    const auto tTopology = aPart.topology();
    return !stk::isQuadrilateralElement(tTopology) && !stk::isHexahedronElement(tTopology) &&
           !stk::is_quad_side(tTopology);
}

namespace detail
{
auto parts_to_selector(const PartReferenceVector& aParts) -> stk::mesh::Selector
{
    return std::accumulate(aParts.cbegin(), aParts.cend(), stk::mesh::Selector{},
                           [](stk::mesh::Selector aSelector, const auto aPart)
                           {
                               aSelector |= aPart.get();
                               return aSelector;
                           });
}
}  // namespace detail
}  // namespace plato::third_party_integration::stk_io
