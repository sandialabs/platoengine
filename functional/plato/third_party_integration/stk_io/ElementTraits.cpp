#include "plato/third_party_integration/stk_io/ElementTraits.hpp"

#include <stk_topology/topology.hpp>

namespace plato::third_party_integration::stk_io
{

namespace detail
{
namespace
{
constexpr auto kNumberOfHexahedronNodes = 8;
constexpr auto kNumberOfTetrahedronNodes = 4;
constexpr auto kNumberOfTriangleNodes = 3;
constexpr auto kNumberOfQuadrilateralNodes = 4;
}  // namespace

template <>
Hexahedron from_coordinates<Hexahedron, kNumberOfHexahedronNodes>(
    const std::array<common::Coordinate, kNumberOfHexahedronNodes>& aCoordinates)
{
    // NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers)
    return Hexahedron{.p000 = aCoordinates[0],
                      .p100 = aCoordinates[1],
                      .p110 = aCoordinates[2],
                      .p010 = aCoordinates[3],
                      .p001 = aCoordinates[4],
                      .p101 = aCoordinates[5],
                      .p111 = aCoordinates[6],
                      .p011 = aCoordinates[7]};
    // NOLINTEND(cppcoreguidelines-avoid-magic-numbers)
}

template <>
Tetrahedron from_coordinates<Tetrahedron, kNumberOfTetrahedronNodes>(
    const std::array<common::Coordinate, kNumberOfTetrahedronNodes>& aCoordinates)
{
    return Tetrahedron{.p0 = aCoordinates[0], .p1 = aCoordinates[1], .p2 = aCoordinates[2], .p3 = aCoordinates[3]};
}

template <>
Triangle from_coordinates<Triangle, kNumberOfTriangleNodes>(
    const std::array<common::Coordinate, kNumberOfTriangleNodes>& aCoordinates)
{
    return Triangle{.p0 = aCoordinates[0], .p1 = aCoordinates[1], .p2 = aCoordinates[2]};
}

template <>
Quadrilateral from_coordinates<Quadrilateral, kNumberOfQuadrilateralNodes>(
    const std::array<common::Coordinate, kNumberOfQuadrilateralNodes>& aCoordinates)
{
    return Quadrilateral{
        .p00 = aCoordinates[0], .p10 = aCoordinates[1], .p11 = aCoordinates[2], .p01 = aCoordinates[3]};
}

}  // namespace detail

}  // namespace plato::third_party_integration::stk_io
