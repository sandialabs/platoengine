#ifndef PLATO_THIRDPARTYINTEGRATION_STKIO_VOLUMEUTILITIES
#define PLATO_THIRDPARTYINTEGRATION_STKIO_VOLUMEUTILITIES

#include <functional>
#include <stk_io/StkMeshIoBroker.hpp>
#include <stk_mesh/base/Bucket.hpp>
#include <stk_topology/topology.hpp>
#include <utility>

#include "plato/third_party_integration/stk_io/TesselationTraits.hpp"
#include "plato/third_party_integration/stk_io/Utilities.hpp"

namespace plato::third_party_integration::stk_io
{

/// @brief Returns the volume of an element @a Element found in bulk data @a aBulk
[[nodiscard]] double element_volume(const stk::mesh::Entity& aElement, const stk::mesh::BulkData& aBulk);

/// @brief Returns the centroid of an element @a Element found in bulk data @a aBulk
[[nodiscard]] common::Coordinate element_centroid(const stk::mesh::Entity& aElement, const stk::mesh::BulkData& aBulk);

/// @brief Return a vector of all the element centroids found in bulk data @a aBulk
[[nodiscard]] std::vector<common::Coordinate> element_centroids(const stk::mesh::BulkData& aBulk);

/// @brief Returns the total volume of a mesh found in bulk data @a aBulk
[[nodiscard]] double mesh_volume(const stk::mesh::BulkData& aBulk);

/// @brief Returns a vector containing the coordinates of an element @a aElement in bulk data @a aBulk
[[nodiscard]] std::vector<common::Coordinate> element_coordinates(const stk::mesh::Entity& aElement,
                                                                  const stk::mesh::BulkData& aBulk);

/// @brief Returns the average nodal density by dividing the total nodes over the total volume of bulk data @a aBulk
[[nodiscard]] double average_nodal_density(const stk::mesh::BulkData& aBulk);

namespace detail
{

template <stk::topology::topology_t... AllTopologies>
using STKTopologySequence = std::integer_sequence<stk::topology::topology_t, AllTopologies...>;
constexpr auto kSupportedTopologies = STKTopologySequence<stk::topology::HEXAHEDRON_8,
                                                          stk::topology::HEXAHEDRON_20,
                                                          stk::topology::TETRAHEDRON_4,
                                                          stk::topology::TETRAHEDRON_10,
                                                          stk::topology::QUADRILATERAL_4,
                                                          stk::topology::TRIANGLE_3,
                                                          stk::topology::TRIANGLE_3_2D,
                                                          stk::topology::QUAD_4_2D,
                                                          stk::topology::SHELL_QUAD_4,
                                                          stk::topology::SHELL_TRIANGLE_3>{};

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

template <typename FunctionTag, stk::topology::topology_t... AllTopologies>
auto element_apply_impl(const stk::mesh::Entity& aElement,
                        const stk::mesh::BulkData& aBulk,
                        const stk::topology::topology_t tTopologyType,
                        const STKTopologySequence<AllTopologies...>)
{
    return (topology_selector_apply<FunctionTag, AllTopologies>(aElement, aBulk, tTopologyType) + ...);
}

template <typename FunctionTag>
auto element_apply(const stk::mesh::Entity& aElement, const stk::mesh::BulkData& aBulk)
{
    return detail::element_apply_impl<FunctionTag>(aElement, aBulk, aBulk.bucket(aElement).topology()(),
                                                   kSupportedTopologies);
}

}  // namespace detail

}  // namespace plato::third_party_integration::stk_io

#endif
