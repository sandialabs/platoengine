#include "plato/third_party_integration/stk_io/ElementTraits.hpp"  // NOLINT(misc-include-cleaner)

#include <array>
#include <cstddef>

#include "plato/third_party_integration/common/Vector3.hpp"
#include "plato/third_party_integration/stk_io/Hexahedron.hpp"
#include "plato/third_party_integration/stk_io/Quadrilateral.hpp"
#include "plato/third_party_integration/stk_io/Tetrahedron.hpp"
#include "plato/third_party_integration/stk_io/Triangle.hpp"

namespace plato::third_party_integration::stk_io::detail
{
namespace
{
constexpr auto kNumberOfHexahedronNodes = std::size_t{8};
constexpr auto kNumberOfTetrahedronNodes = std::size_t{4};
constexpr auto kNumberOfTriangleNodes = std::size_t{3};
constexpr auto kNumberOfQuadrilateralNodes = std::size_t{4};
}  // namespace

template <>
auto from_coordinates<Hexahedron, kNumberOfHexahedronNodes>(
    const std::array<common::Coordinate, kNumberOfHexahedronNodes>& aCoordinates) -> Hexahedron
{
    // NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)
    return Hexahedron{.mP000 = aCoordinates[0],
                      .mP100 = aCoordinates[1],
                      .mP110 = aCoordinates[2],
                      .mP010 = aCoordinates[3],
                      .mP001 = aCoordinates[4],
                      .mP101 = aCoordinates[5],
                      .mP111 = aCoordinates[6],
                      .mP011 = aCoordinates[7]};
    // NOLINTEND(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)
}

template <>
auto from_coordinates<Tetrahedron, kNumberOfTetrahedronNodes>(
    const std::array<common::Coordinate, kNumberOfTetrahedronNodes>& aCoordinates) -> Tetrahedron
{
    return Tetrahedron{.mP0 = aCoordinates[0], .mP1 = aCoordinates[1], .mP2 = aCoordinates[2], .mP3 = aCoordinates[3]};
}

template <>
auto from_coordinates<Triangle, kNumberOfTriangleNodes>(
    const std::array<common::Coordinate, kNumberOfTriangleNodes>& aCoordinates) -> Triangle
{
    return Triangle{.mP0 = aCoordinates[0], .mP1 = aCoordinates[1], .mP2 = aCoordinates[2]};
}

template <>
auto from_coordinates<Quadrilateral, kNumberOfQuadrilateralNodes>(
    const std::array<common::Coordinate, kNumberOfQuadrilateralNodes>& aCoordinates) -> Quadrilateral
{
    return Quadrilateral{
        .mP00 = aCoordinates[0], .mP10 = aCoordinates[1], .mP11 = aCoordinates[2], .mP01 = aCoordinates[3]};
}

}  // namespace plato::third_party_integration::stk_io::detail
