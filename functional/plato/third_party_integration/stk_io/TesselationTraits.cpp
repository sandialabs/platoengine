#include "plato/third_party_integration/stk_io/TesselationTraits.hpp"

#include <numeric>
#include <stk_topology/topology.hpp>

namespace plato::third_party_integration::stk_io
{

namespace detail
{

template <>
Tetrahedron from_coordinates<Tetrahedron>(const Tetrahedron::Indices& aIndices,
                                          const std::vector<common::Coordinate>& aCoordinates)
{
    assert(aCoordinates.size() > *std::max_element(aIndices.begin(), aIndices.end()));
    return Tetrahedron{/*.p0 = */ aCoordinates[aIndices[0]],
                       /*.p1 = */ aCoordinates[aIndices[1]],
                       /*.p2 = */ aCoordinates[aIndices[2]],
                       /*.p3 = */ aCoordinates[aIndices[3]]};
}

template <>
Triangle from_coordinates<Triangle>(const Triangle::Indices& aIndices,
                                    const std::vector<common::Coordinate>& aCoordinates)
{
    assert(aCoordinates.size() > *std::max_element(aIndices.begin(), aIndices.end()));
    return Triangle{/*.p0 = */ aCoordinates[aIndices[0]],
                    /*.p1 = */ aCoordinates[aIndices[1]],
                    /*.p2 = */ aCoordinates[aIndices[2]]};
}

}  // namespace detail

}  // namespace plato::third_party_integration::stk_io
