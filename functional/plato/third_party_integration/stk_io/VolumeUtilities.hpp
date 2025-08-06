#ifndef PLATO_THIRDPARTYINTEGRATION_STKIO_VOLUMEUTILITIES
#define PLATO_THIRDPARTYINTEGRATION_STKIO_VOLUMEUTILITIES

#include <array>
#include <functional>
#include <stk_io/StkMeshIoBroker.hpp>
#include <stk_mesh/base/Bucket.hpp>
#include <stk_topology/topology.hpp>
#include <utility>

#include "plato/third_party_integration/stk_io/ReadUtilities.hpp"
#include "plato/third_party_integration/stk_io/TesselationTraits.hpp"

namespace plato::third_party_integration::stk_io
{

/// @brief Returns the volume of an element @a Element found in bulk data @a aBulk
[[nodiscard]] double element_volume(const stk::mesh::Entity& aElement, const stk::mesh::BulkData& aBulk);

/// @brief Returns the centroid of an element @a Element found in bulk data @a aBulk
[[nodiscard]] common::Coordinate element_centroid(const stk::mesh::Entity& aElement, const stk::mesh::BulkData& aBulk);

/// @brief Returns the maximum edge length of an element @a Element found in bulk data @a aBulk
[[nodiscard]] double element_max_edge_length(const stk::mesh::Entity& aElement, const stk::mesh::BulkData& aBulk);

/// @brief Return a vector of all the element centroids found in bulk data @a aBulk
[[nodiscard]] std::vector<common::Coordinate> element_centroids(const stk::mesh::BulkData& aBulk);

/// @brief Returns a vector of the element centroids associated with the parts given in @a aParts.
[[nodiscard]] std::vector<common::Coordinate> element_centroids(const stk::mesh::BulkData& aBulk,
                                                                const PartReferenceVector& aParts);

/// @brief Returns the total volume of a mesh found in bulk data @a aBulk
[[nodiscard]] double mesh_volume(const stk::mesh::BulkData& aBulk);

/// @brief Returns a vector containing the coordinates of an element @a aElement in bulk data @a aBulk
[[nodiscard]] std::vector<common::Coordinate> element_coordinates(const stk::mesh::Entity& aElement,
                                                                  const stk::mesh::BulkData& aBulk);

/// @brief Returns the average element maximum edge length over all the elements found in bulk data @a aBulk
[[nodiscard]] double average_element_max_edge_length(const stk::mesh::BulkData& aBulk);

namespace detail
{

struct VolumeTag
{
};

struct CentroidTag
{
};

template <stk::topology::topology_t Topology>
struct ApplyTaggedFunction
{
    double operator()(const std::vector<common::Coordinate>& aCoordinates, VolumeTag) const
    {
        return detail::volume_impl<Topology>(aCoordinates);
    }

    common::Coordinate operator()(const std::vector<common::Coordinate>& aCoordinates, CentroidTag) const
    {
        return detail::centroid_impl<Topology>(aCoordinates);
    }

    template <typename FunctionTag>
    static auto zero()
    {
        using ReturnType =
            std::invoke_result_t<ApplyTaggedFunction<Topology>, std::vector<common::Coordinate>, FunctionTag>;
        return ReturnType{};
    }
};

template <typename FunctionTag, stk::topology::topology_t Topology>
auto topology_selector_apply(const stk::mesh::Entity& aElement,
                             const stk::mesh::BulkData& aBulk,
                             const stk::topology::topology_t tTopologyType)
{
    if (tTopologyType == Topology)
    {
        const std::vector<common::Coordinate> tCoordinates = element_coordinates(aElement, aBulk);
        return ApplyTaggedFunction<Topology>{}(tCoordinates, FunctionTag{});
    }

    return ApplyTaggedFunction<Topology>::template zero<FunctionTag>();
}

template <typename FunctionTag>
auto element_apply(const stk::mesh::Entity& aElement, const stk::mesh::BulkData& aBulk)
{
    constexpr auto tSupportedTopologies =
        std::array{stk::topology::HEXAHEDRON_8,    stk::topology::HEXAHEDRON_20,   stk::topology::TETRAHEDRON_4,
                   stk::topology::TETRAHEDRON_10,  stk::topology::QUADRILATERAL_4, stk::topology::TRIANGLE_3,
                   stk::topology::TRIANGLE_3_2D,   stk::topology::QUAD_4_2D,       stk::topology::SHELL_QUAD_4,
                   stk::topology::SHELL_TRIANGLE_3};

    return [&aElement, &aBulk, &tSupportedTopologies]<std::size_t... AllTopologiesIndices>(
               const std::integer_sequence<std::size_t, AllTopologiesIndices...>)
    {
        return (topology_selector_apply<FunctionTag, tSupportedTopologies[AllTopologiesIndices]>(
                    aElement, aBulk, aBulk.bucket(aElement).topology()()) +
                ...);
    }(std::make_index_sequence<tSupportedTopologies.size()>());
}

}  // namespace detail

}  // namespace plato::third_party_integration::stk_io

#endif
