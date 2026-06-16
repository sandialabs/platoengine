#include "plato/third_party_integration/stk_io/VolumeUtilities.hpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <functional>
#include <iterator>
#include <numeric>
#include <ranges>
#include <span>
#include <stk_mesh/base/Bucket.hpp>
#include <stk_mesh/base/Entity.hpp>
#include <stk_mesh/base/FieldBase.hpp>
#include <stk_mesh/base/MetaData.hpp>
#include <stk_mesh/base/Types.hpp>
#include <stk_topology/topology_decl.hpp>
#include <type_traits>
#include <utility>
#include <vector>

#include "plato/third_party_integration/common/Vector3.hpp"
#include "plato/third_party_integration/stk_io/ElementTraits.hpp"
#include "plato/third_party_integration/stk_io/MeshFieldOperations.hpp"
#include "plato/third_party_integration/stk_io/ReadUtilities.hpp"
#include "plato/utilities/PairWiseAccumulate.hpp"

namespace plato::third_party_integration::stk_io
{
namespace
{
[[nodiscard]] auto coordinate_from_data_array(const std::span<const double> aCoordinate) -> common::Coordinate
{
    if (aCoordinate.size() == 3U)
    {
        return common::Coordinate{.x = aCoordinate[0], .y = aCoordinate[1], .z = aCoordinate[2]};
    }

    return common::Coordinate{.x = aCoordinate[0], .y = aCoordinate[1], .z = 0};
}

auto get_edge_nodes(const stk::topology& aElementTopology,
                    const stk::mesh::Entity* const aElemNodes,
                    const unsigned int aEdgeOrdinal) -> stk::mesh::EntityVector
{
    const stk::topology tEdgeTopology = aElementTopology.edge_topology(aEdgeOrdinal);
    stk::mesh::EntityVector tEdgeNodes(tEdgeTopology.num_nodes());
    aElementTopology.edge_nodes(aElemNodes, aEdgeOrdinal, tEdgeNodes.data());
    return tEdgeNodes;
}

auto compute_edge_length(const stk::mesh::BulkData& aBulk, const stk::mesh::EntityVector& aEdgeNodes) -> double
{
    assert(aEdgeNodes.size() >= 2);
    const unsigned int tNumDimensions = spatial_dimensions(aBulk);
    const stk::mesh::FieldBase* const tCoords = aBulk.mesh_meta_data().coordinate_field();

    constexpr unsigned int tVertex1Index = 0;
    const common::Coordinate tCoord1 = coordinate_from_data_array(std::span(
        static_cast<const double*>(stk::mesh::field_data(*tCoords, aEdgeNodes[tVertex1Index])), tNumDimensions));

    constexpr unsigned int tVertex2Index = 1;
    const common::Coordinate tCoord2 = coordinate_from_data_array(std::span(
        static_cast<const double*>(stk::mesh::field_data(*tCoords, aEdgeNodes[tVertex2Index])), tNumDimensions));

    return common::magnitude(tCoord2 - tCoord1);
}

template <typename F>
[[nodiscard]] auto zero()
{
    using ReturnType = std::invoke_result_t<F, std::vector<common::Coordinate>>;
    return ReturnType{};
}

template <template <stk::topology::topology_t> typename Function, stk::topology::topology_t kTopology>
auto topology_selector_apply(const stk::mesh::Entity& aElement,
                             const stk::mesh::BulkData& aBulk,
                             const stk::topology::topology_t aTopologyType)
{
    if (aTopologyType == kTopology)
    {
        return Function<kTopology>{}(element_coordinates(aElement, aBulk));
    }

    return zero<Function<kTopology>>();
}

/// @brief overloaded operator+ so that element_apply works for return types of std::vector<Vector3>. Is implemented
/// specifically to work with how element_apply is implemented.
auto operator+(const ElementNodalSensitivities& aLeftHandSideVector,
               const ElementNodalSensitivities& aRightHandSideVector) -> ElementNodalSensitivities
{
    if (!aLeftHandSideVector.mValue.empty() && aRightHandSideVector.mValue.empty())
    {
        return {aLeftHandSideVector};
    }
    if (aLeftHandSideVector.mValue.empty() && !aRightHandSideVector.mValue.empty())
    {
        return {aRightHandSideVector};
    }

    return {};
}

template <template <stk::topology::topology_t> typename Function>
auto element_apply(const stk::mesh::Entity& aElement, const stk::mesh::BulkData& aBulk)
{
    constexpr auto tSupportedTopologies = std::array<stk::topology::topology_t, 10U>{
        stk::topology::HEXAHEDRON_8,    stk::topology::HEXAHEDRON_20,   stk::topology::TETRAHEDRON_4,
        stk::topology::TETRAHEDRON_10,  stk::topology::QUADRILATERAL_4, stk::topology::TRIANGLE_3,
        stk::topology::TRIANGLE_3_2D,   stk::topology::QUAD_4_2D,       stk::topology::SHELL_QUAD_4,
        stk::topology::SHELL_TRIANGLE_3};

    return [&aElement, &aBulk, &tSupportedTopologies]<std::size_t... kAllTopologiesIndices>(
               const std::integer_sequence<std::size_t, kAllTopologiesIndices...>)
    {
        return (topology_selector_apply<Function, tSupportedTopologies[kAllTopologiesIndices]>(
                    aElement, aBulk, aBulk.bucket(aElement).topology()()) +
                ...);
    }(std::make_index_sequence<tSupportedTopologies.size()>());
}

template <stk::topology::topology_t kTopology>
struct VolumeFunction
{
    auto operator()(const std::vector<common::Coordinate>& aCoordinates) const -> double
    {
        return detail::volume_impl<kTopology>(aCoordinates);
    }
};

template <stk::topology::topology_t kTopology>
struct VolumeNodalSensitivitiesFunction
{
    auto operator()(const std::vector<common::Coordinate>& aCoordinates) const -> ElementNodalSensitivities
    {
        const auto tSensitivityArray = detail::volume_nodal_sensitivities_impl<kTopology>(aCoordinates);
        return ElementNodalSensitivities{
            std::vector<common::Vector3>(tSensitivityArray.begin(), tSensitivityArray.end())};
    }
};

template <stk::topology::topology_t kTopology>
struct CentroidFunction
{
    auto operator()(const std::vector<common::Coordinate>& aCoordinates) const -> common::Coordinate
    {
        return detail::centroid_impl<kTopology>(aCoordinates);
    }
};

void assemble_element_volume_nodal_sensitivity(const stk::mesh::BulkData& aBulk,
                                               const stk::mesh::Entity& aElement,
                                               const std::vector<std::size_t>& aGlobalNodeIDs,
                                               std::vector<common::Vector3>& aNodalSensitivities)
{
    const auto tElementNodalSensitivities = stk_io::element_volume_nodal_sensitivities(aElement, aBulk);
    std::for_each(aBulk.begin_nodes(aElement), aBulk.end_nodes(aElement),
                  [&aBulk, &tElementNodalSensitivities, &aGlobalNodeIDs, &aNodalSensitivities,
                   tCounter{0}](const stk::mesh::Entity& aNode) mutable
                  {
                      const auto tLocalIndex =
                          stk_io::detail::global_to_local_index(aGlobalNodeIDs, aBulk.entity_key(aNode).id());
                      aNodalSensitivities[tLocalIndex] =
                          aNodalSensitivities[tLocalIndex] + tElementNodalSensitivities.mValue[tCounter++];
                  });
}

}  // namespace

auto element_volume(const stk::mesh::Entity& aElement, const stk::mesh::BulkData& aBulk) -> double
{
    return element_apply<VolumeFunction>(aElement, aBulk);
}

auto element_centroid(const stk::mesh::Entity& aElement, const stk::mesh::BulkData& aBulk) -> common::Coordinate
{
    return element_apply<CentroidFunction>(aElement, aBulk);
}

auto element_volume_nodal_sensitivities(const stk::mesh::Entity& aElement, const stk::mesh::BulkData& aBulk)
    -> ElementNodalSensitivities
{
    return element_apply<VolumeNodalSensitivitiesFunction>(aElement, aBulk);
}

auto element_max_edge_length(const stk::mesh::Entity& aElement, const stk::mesh::BulkData& aBulk) -> double
{
    const stk::mesh::Entity* tElemNodes = aBulk.begin(aElement, stk::topology::NODE_RANK);
    const stk::topology tElementTopology = aBulk.bucket(aElement).topology();
    const unsigned tNumEdgesPerElement = tElementTopology.num_edges();

    std::vector<double> tEdgeLengths;
    tEdgeLengths.reserve(tNumEdgesPerElement);
    for (unsigned int tEdgeOrdinal = 0; tEdgeOrdinal < tNumEdgesPerElement; ++tEdgeOrdinal)
    {
        const auto tEdgeNodes = get_edge_nodes(tElementTopology, tElemNodes, tEdgeOrdinal);
        tEdgeLengths.push_back(compute_edge_length(aBulk, tEdgeNodes));
    }

    const auto tMaxIterator = std::max_element(tEdgeLengths.cbegin(), tEdgeLengths.cend());
    return tMaxIterator == tEdgeLengths.end() ? 0 : *tMaxIterator;
}

auto element_centroids(const stk::mesh::BulkData& aBulk) -> std::vector<common::Coordinate>
{
    return element_centroids(aBulk, PartReferenceVector{std::cref(aBulk.mesh_meta_data().universal_part())});
}

auto element_centroids(const stk::mesh::BulkData& aBulk, const PartReferenceVector& aParts)
    -> std::vector<common::Coordinate>
{
    const stk::mesh::EntityVector tElements = element_vector(aBulk, aParts);
    std::vector<common::Coordinate> tCentroids;
    tCentroids.reserve(tElements.size());
    std::transform(tElements.begin(), tElements.end(), std::back_inserter(tCentroids),
                   [&aBulk](const auto& aElement) { return element_centroid(aElement, aBulk); });

    return tCentroids;
}

auto mesh_volume(const stk::mesh::BulkData& aBulk) -> double
{
    const auto tElements = element_vector(aBulk);

    const auto tElementVolumes =
        tElements | std::views::transform([&aBulk](const auto& aElement) { return element_volume(aElement, aBulk); });

    return utilities::pair_wise_accumulate(tElementVolumes);
}

auto volume_nodal_sensitivities(const stk::mesh::BulkData& aBulk, const PartReferenceVector& aParts)
    -> std::vector<third_party_integration::common::Vector3>
{
    std::vector<common::Vector3> tNodalSensitivities(stk_io::node_size(aBulk));
    const auto tGlobalNodeIDs = node_ids(aBulk, aBulk.mesh_meta_data().universal_part());
    for (const auto& tPart : aParts)
    {
        const auto tElements = stk_io::element_vector(aBulk, tPart.get());
        for (const auto& tElement : tElements)
        {
            assemble_element_volume_nodal_sensitivity(aBulk, tElement, tGlobalNodeIDs, tNodalSensitivities);
        }
    }
    return tNodalSensitivities;
}

auto element_coordinates(const stk::mesh::Entity& aElement, const stk::mesh::BulkData& aBulk)
    -> std::vector<common::Coordinate>
{
    std::vector<common::Coordinate> tCoordinates;
    const unsigned int tNumDimensions = spatial_dimensions(aBulk);
    const stk::mesh::FieldBase* const tCoords = aBulk.mesh_meta_data().coordinate_field();
    std::transform(aBulk.begin_nodes(aElement), aBulk.end_nodes(aElement), std::back_inserter(tCoordinates),
                   [tNumDimensions, tCoords](const stk::mesh::Entity& aNode)
                   {
                       const auto* const tData = static_cast<const double*>(stk::mesh::field_data(*tCoords, aNode));
                       return coordinate_from_data_array(std::span(tData, tNumDimensions));
                   });
    return tCoordinates;
}

auto average_element_max_edge_length(const stk::mesh::BulkData& aBulk) -> double
{
    const stk::mesh::EntityVector tElements = element_vector(aBulk);
    return std::accumulate(tElements.cbegin(), tElements.cend(), 0.0, [&aBulk](const double aSum, const auto& aElement)
                           { return aSum + element_max_edge_length(aElement, aBulk); }) /
           static_cast<double>(tElements.size());
}

}  // namespace plato::third_party_integration::stk_io
