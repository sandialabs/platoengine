#include "plato/third_party_integration/stk_io/ElementTraits.hpp"  // NOLINT(misc-include-cleaner)

#include <array>
#include <cstddef>

#include "plato/third_party_integration/common/Vector3.hpp"
#include "plato/third_party_integration/stk_io/Hexahedron.hpp"
#include "plato/third_party_integration/stk_io/Quadrilateral.hpp"
#include "plato/third_party_integration/stk_io/Tetrahedron.hpp"
#include "plato/third_party_integration/stk_io/Triangle.hpp"

namespace plato::third_party_integration::stk_io
{

namespace detail
{
namespace
{
constexpr auto kNumberOfHexahedronNodes = std::size_t{8};
constexpr auto kNumberOfTetrahedronNodes = std::size_t{4};
constexpr auto kNumberOfTriangleNodes = std::size_t{3};
constexpr auto kNumberOfQuadrilateralNodes = std::size_t{4};
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
