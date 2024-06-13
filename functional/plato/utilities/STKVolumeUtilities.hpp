#ifndef PLATO_UTILITIES_STKVOLUMEUTILITIES
#define PLATO_UTILITIES_STKVOLUMEUTILITIES

#include <stk_io/StkMeshIoBroker.hpp>
#include <stk_mesh/base/Bucket.hpp>
#include <stk_topology/topology.hpp>

#include "plato/utilities/STKUtilities.hpp"
#include "plato/utilities/TesselationTraits.hpp"
#include "plato/utilities/Vector3.hpp"

namespace plato::utilities
{

double element_volume(const stk::mesh::Entity& aElement, const stk::mesh::BulkData& aBulk);

double mesh_volume(const std::filesystem::path& aMeshFileName);

std::vector<Coordinate> element_coordinates(const stk::mesh::Entity& aElement, const stk::mesh::BulkData& aBulk);

namespace detail
{
template <stk::topology::topology_t Topology>
double topology_selector(const stk::mesh::Entity& aElement,
                         const stk::mesh::BulkData& aBulk,
                         const stk::topology::topology_t tTopologyType)
{
    const std::vector<Coordinate> tCoordinates = element_coordinates(aElement, aBulk);

    if (tTopologyType == Topology)
    {
        // using TraitType = TesselationTraits<Topology>;
        return detail::volume_impl<Topology>(tCoordinates);
    }
    return 0;
}

template <stk::topology::topology_t... AllTopologies>
double element_volume_impl(const stk::mesh::Entity& aElement,
                           const stk::mesh::BulkData& aBulk,
                           const stk::topology::topology_t tTopologyType)
{
    const std::vector<Coordinate> tCoordinates = element_coordinates(aElement, aBulk);
    return (topology_selector<AllTopologies>(aElement, aBulk, tTopologyType) + ...);
}

}  // namespace detail

}  // namespace plato::utilities

#endif
