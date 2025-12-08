#include "plato/third_party_integration/stk_io/VolumeUtilities.hpp"

#include <algorithm>
#include <numeric>
#include <stk_mesh/base/Bucket.hpp>
#include <stk_mesh/base/FieldBase.hpp>
#include <stk_mesh/base/MetaData.hpp>
#include <stk_topology/topology.hpp>
#include <vector>

#include "plato/third_party_integration/stk_io/TesselationTraits.hpp"
#include "plato/utilities/PairWiseAccumulate.hpp"

namespace plato::third_party_integration::stk_io
{
namespace
{

common::Coordinate coordinate_from_data_array(const double* aData, const unsigned int aNumDimensions)
{
    if (aNumDimensions == 3)
    {
        return common::Coordinate{/*.x=*/aData[0],
                                  /*.y=*/aData[1],
                                  /*.z=*/aData[2]};
    }
    else
    {
        return common::Coordinate{/*.x=*/aData[0],
                                  /*.y=*/aData[1],
                                  /*.z=*/0};
    }
}

stk::mesh::EntityVector get_edge_nodes(const stk::topology& aElementTopology,
                                       const stk::mesh::Entity* const aElemNodes,
                                       const unsigned int aEdgeOrdinal)
{
    const stk::topology tEdgeTopology = aElementTopology.edge_topology(aEdgeOrdinal);
    stk::mesh::EntityVector tEdgeNodes(tEdgeTopology.num_nodes());
    aElementTopology.edge_nodes(&aElemNodes[0], aEdgeOrdinal, tEdgeNodes.data());
    return tEdgeNodes;
}

double compute_edge_length(const stk::mesh::BulkData& aBulk, const stk::mesh::EntityVector& aEdgeNodes)
{
    assert(aEdgeNodes.size() >= 2);
    const unsigned int tNumDimensions = spatial_dimensions(aBulk);
    const stk::mesh::FieldBase* const tCoords = aBulk.mesh_meta_data().coordinate_field();

    constexpr unsigned int tVertex1Index = 0;
    const common::Coordinate tCoord1 = coordinate_from_data_array(
        static_cast<const double*>(stk::mesh::field_data(*tCoords, aEdgeNodes[tVertex1Index])), tNumDimensions);

    constexpr unsigned int tVertex2Index = 1;
    const common::Coordinate tCoord2 = coordinate_from_data_array(
        static_cast<const double*>(stk::mesh::field_data(*tCoords, aEdgeNodes[tVertex2Index])), tNumDimensions);

    return common::magnitude(tCoord2 - tCoord1);
}

template <typename F>
[[nodiscard]] auto zero()
{
    using ReturnType = std::invoke_result_t<F, std::vector<common::Coordinate>>;
    return ReturnType{};
}

template <template <stk::topology::topology_t> typename Function, stk::topology::topology_t Topology>
auto topology_selector_apply(const stk::mesh::Entity& aElement,
                             const stk::mesh::BulkData& aBulk,
                             const stk::topology::topology_t tTopologyType)
{
    if (tTopologyType == Topology)
    {
        return Function<Topology>{}(element_coordinates(aElement, aBulk));
    }

    return zero<Function<Topology>>();
}

template <template <stk::topology::topology_t> typename Function>
auto element_apply(const stk::mesh::Entity& aElement, const stk::mesh::BulkData& aBulk)
{
    constexpr auto tSupportedTopologies = std::array<stk::topology::topology_t, 10U>{
        stk::topology::HEXAHEDRON_8,    stk::topology::HEXAHEDRON_20,   stk::topology::TETRAHEDRON_4,
        stk::topology::TETRAHEDRON_10,  stk::topology::QUADRILATERAL_4, stk::topology::TRIANGLE_3,
        stk::topology::TRIANGLE_3_2D,   stk::topology::QUAD_4_2D,       stk::topology::SHELL_QUAD_4,
        stk::topology::SHELL_TRIANGLE_3};

    return [&aElement, &aBulk, &tSupportedTopologies]<std::size_t... AllTopologiesIndices>(
               const std::integer_sequence<std::size_t, AllTopologiesIndices...>)
    {
        return (topology_selector_apply<Function, tSupportedTopologies[AllTopologiesIndices]>(
                    aElement, aBulk, aBulk.bucket(aElement).topology()()) +
                ...);
    }(std::make_index_sequence<tSupportedTopologies.size()>());
}

template <stk::topology::topology_t Topology>
struct VolumeFunction
{
    double operator()(const std::vector<common::Coordinate>& aCoordinates) const
    {
        return detail::volume_impl<Topology>(aCoordinates);
    }
};

template <stk::topology::topology_t Topology>
struct CentroidFunction
{
    common::Coordinate operator()(const std::vector<common::Coordinate>& aCoordinates) const
    {
        return detail::centroid_impl<Topology>(aCoordinates);
    }
};

}  // namespace

double element_volume(const stk::mesh::Entity& aElement, const stk::mesh::BulkData& aBulk)
{
    return element_apply<VolumeFunction>(aElement, aBulk);
}

common::Coordinate element_centroid(const stk::mesh::Entity& aElement, const stk::mesh::BulkData& aBulk)
{
    return element_apply<CentroidFunction>(aElement, aBulk);
}

double element_max_edge_length(const stk::mesh::Entity& aElement, const stk::mesh::BulkData& aBulk)
{
    const stk::mesh::Entity* tElemNodes = aBulk.begin(aElement, stk::topology::NODE_RANK);
    const stk::topology tElementTopology = aBulk.bucket(aElement).topology();
    const unsigned tNumEdgesPerElement = tElementTopology.num_edges();

    std::vector<double> tEdgeLengths;
    tEdgeLengths.reserve(tNumEdgesPerElement);
    for (unsigned int aEdgeOrdinal = 0; aEdgeOrdinal < tNumEdgesPerElement; ++aEdgeOrdinal)
    {
        const auto tEdgeNodes = get_edge_nodes(tElementTopology, tElemNodes, aEdgeOrdinal);
        tEdgeLengths.push_back(compute_edge_length(aBulk, tEdgeNodes));
    }

    const auto tMaxIterator = std::max_element(tEdgeLengths.cbegin(), tEdgeLengths.cend());
    return tMaxIterator == tEdgeLengths.end() ? 0 : *tMaxIterator;
}

double mesh_volume(const stk::mesh::BulkData& aBulk)
{
    const stk::mesh::EntityVector tElements = element_vector(aBulk);
    std::vector<double> tVolume;
    tVolume.reserve(tElements.size());
    std::transform(tElements.begin(), tElements.end(), std::back_inserter(tVolume),
                   [&aBulk](const auto& iElement) { return element_volume(iElement, aBulk); });

    return utilities::pair_wise_accumulate(tVolume);
}

std::vector<common::Coordinate> element_centroids(const stk::mesh::BulkData& aBulk)
{
    return element_centroids(aBulk, PartReferenceVector{std::cref(aBulk.mesh_meta_data().universal_part())});
}

std::vector<common::Coordinate> element_centroids(const stk::mesh::BulkData& aBulk, const PartReferenceVector& aParts)
{
    const stk::mesh::EntityVector tElements = element_vector(aBulk, aParts);
    std::vector<common::Coordinate> tCentroids;
    tCentroids.reserve(tElements.size());
    std::transform(tElements.begin(), tElements.end(), std::back_inserter(tCentroids),
                   [&aBulk](const auto& iElement) { return element_centroid(iElement, aBulk); });

    return tCentroids;
}

std::vector<common::Coordinate> element_coordinates(const stk::mesh::Entity& aElement, const stk::mesh::BulkData& aBulk)
{
    std::vector<common::Coordinate> tCoordinates;
    const unsigned int tNumDimensions = spatial_dimensions(aBulk);
    const stk::mesh::FieldBase* const tCoords = aBulk.mesh_meta_data().coordinate_field();
    std::transform(aBulk.begin_nodes(aElement), aBulk.end_nodes(aElement), std::back_inserter(tCoordinates),
                   [tNumDimensions, tCoords](const stk::mesh::Entity& node)
                   {
                       const auto tData = static_cast<const double*>(stk::mesh::field_data(*tCoords, node));
                       return coordinate_from_data_array(tData, tNumDimensions);
                   });
    return tCoordinates;
}

double average_element_max_edge_length(const stk::mesh::BulkData& aBulk)
{
    const stk::mesh::EntityVector tElements = element_vector(aBulk);
    return std::accumulate(tElements.cbegin(), tElements.cend(), 0.0, [&aBulk](const double aSum, const auto& iElement)
                           { return aSum + element_max_edge_length(iElement, aBulk); }) /
           static_cast<double>(tElements.size());
}

}  // namespace plato::third_party_integration::stk_io
