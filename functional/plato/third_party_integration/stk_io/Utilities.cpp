#include "plato/third_party_integration/stk_io/Utilities.hpp"

#include <algorithm>
#include <numeric>
#include <stk_mesh/base/BulkData.hpp>
#include <stk_mesh/base/Comm.hpp>
#include <stk_mesh/base/Field.hpp>
#include <stk_mesh/base/GetEntities.hpp>
#include <stk_mesh/base/MetaData.hpp>
#include <stk_topology/topology.hpp>
#include <stk_util/parallel/Parallel.hpp>

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

}  // namespace

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
