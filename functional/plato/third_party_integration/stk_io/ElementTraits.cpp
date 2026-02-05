#include "plato/third_party_integration/stk_io/ElementTraits.hpp"

#include <stk_topology/topology.hpp>

namespace plato::third_party_integration::stk_io
{

namespace detail
{

template <>
Hexahedron from_coordinates<Hexahedron, 8>(const std::array<common::Coordinate, 8>& aCoordinates)
{
    return Hexahedron{.p000 = aCoordinates[0],
                      .p100 = aCoordinates[1],
                      .p110 = aCoordinates[2],
                      .p010 = aCoordinates[3],
                      .p001 = aCoordinates[4],
                      .p101 = aCoordinates[5],
                      .p111 = aCoordinates[6],
                      .p011 = aCoordinates[7]};
}

template <>
Tetrahedron from_coordinates<Tetrahedron, 4>(const std::array<common::Coordinate, 4>& aCoordinates)
{
    return Tetrahedron{.p0 = aCoordinates[0], .p1 = aCoordinates[1], .p2 = aCoordinates[2], .p3 = aCoordinates[3]};
}

template <>
Triangle from_coordinates<Triangle, 3>(const std::array<common::Coordinate, 3>& aCoordinates)
{
    return Triangle{.p0 = aCoordinates[0], .p1 = aCoordinates[1], .p2 = aCoordinates[2]};
}

template <>
Quadrilateral from_coordinates<Quadrilateral, 4>(const std::array<common::Coordinate, 4>& aCoordinates)
{
    return Quadrilateral{
        .p00 = aCoordinates[0], .p10 = aCoordinates[1], .p11 = aCoordinates[2], .p01 = aCoordinates[3]};
}

}  // namespace detail

}  // namespace plato::third_party_integration::stk_io
