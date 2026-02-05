#ifndef PLATO_UTILITIES_ELEMENTTRAITS
#define PLATO_UTILITIES_ELEMENTTRAITS

#include <cassert>
#include <stk_topology/topology.hpp>
#include <vector>

#include "plato/third_party_integration/stk_io/Hexahedron.hpp"
#include "plato/third_party_integration/stk_io/Quadrilateral.hpp"
#include "plato/third_party_integration/stk_io/Tetrahedron.hpp"
#include "plato/third_party_integration/stk_io/Triangle.hpp"
namespace plato::third_party_integration::stk_io
{

/// @brief ElementTraits is a templated struct that stores all the information defining how an element type
/// from STK relates to its geometric primitive. ElementType specifies the geometric primitive, e.g. Triangle,
/// Tetrahedron. kNumNodes the number of nodes in the element.
template <stk::topology::topology_t kStkTopology>
struct ElementTraits
{
};

template <>
struct ElementTraits<stk::topology::HEXAHEDRON_8>
{
    constexpr static auto kNumNodes = 8;
    using ElementType = Hexahedron;
};

template <>
struct ElementTraits<stk::topology::HEXAHEDRON_20> : public ElementTraits<stk::topology::HEXAHEDRON_8>
{
};

template <>
struct ElementTraits<stk::topology::TETRAHEDRON_4>
{
    constexpr static auto kNumNodes = 4;
    using ElementType = Tetrahedron;
};

template <>
struct ElementTraits<stk::topology::TETRAHEDRON_10> : public ElementTraits<stk::topology::TETRAHEDRON_4>
{
};

template <>
struct ElementTraits<stk::topology::QUAD_4>
{
    constexpr static auto kNumNodes = 4;
    using ElementType = Quadrilateral;
};

template <>
struct ElementTraits<stk::topology::SHELL_QUAD_4> : public ElementTraits<stk::topology::QUAD_4>
{
};

template <>
struct ElementTraits<stk::topology::QUAD_4_2D> : public ElementTraits<stk::topology::QUAD_4>
{
};

template <>
struct ElementTraits<stk::topology::TRIANGLE_3>
{
    constexpr static auto kNumNodes = 3;
    using ElementType = Triangle;
};

template <>
struct ElementTraits<stk::topology::TRIANGLE_3_2D> : public ElementTraits<stk::topology::TRIANGLE_3>
{
};

template <>
struct ElementTraits<stk::topology::SHELL_TRIANGLE_3> : public ElementTraits<stk::topology::TRIANGLE_3>
{
};

namespace detail
{
/// @brief copies the coordinates in @a aCoordinates into an array with size @tparam NumberOfNodes
template <stk::topology::topology_t kStkTopology, std::size_t NumberOfNodes>
[[nodiscard]] auto coordinate_vector_to_array(const std::vector<common::Coordinate>& aCoordinates)
    -> std::array<common::Coordinate, NumberOfNodes>
{
    std::array<common::Coordinate, ElementTraits<kStkTopology>::kNumNodes> tCoordinateArray;
    std::copy_n(aCoordinates.begin(), ElementTraits<kStkTopology>::kNumNodes, tCoordinateArray.begin());
    return tCoordinateArray;
}

/// @brief concept to restrict the return type of from_coordinates to have the operations of a geometric primitive.
template <typename T>
concept GeometricPrimitiveType = requires(T aPrimitive) {
    aPrimitive.volume();
    aPrimitive.centroid();
    aPrimitive.volumeVertexSensitivities();
};

/// @brief Create a ReturnObject from the coordinates specified in @a aCoordinates according to the tesselation indices
/// @a aIndices
template <GeometricPrimitiveType ReturnObject, std::size_t NumberOfNodes>
ReturnObject from_coordinates(const std::array<common::Coordinate, NumberOfNodes>& aCoordinates);

/// @brief Determine the volume of a single element with coordinates @a aCoordinates as dictated by the Tesselation
/// TraitType
template <stk::topology::topology_t kStkTopology>
double volume_impl(const std::vector<common::Coordinate>& aCoordinates)
{
    using ElementType = ElementTraits<kStkTopology>::ElementType;
    const ElementType tElement = from_coordinates<ElementType, ElementTraits<kStkTopology>::kNumNodes>(
        coordinate_vector_to_array<kStkTopology, ElementTraits<kStkTopology>::kNumNodes>(aCoordinates));
    return tElement.volume();
}

/// @brief Determine the centroid of a single element with coordinates @a aCoordinates as dictated by the Tesselation
/// TraitType
template <stk::topology::topology_t kStkTopology>
common::Coordinate centroid_impl(const std::vector<common::Coordinate>& aCoordinates)
{
    using ElementType = ElementTraits<kStkTopology>::ElementType;
    const ElementType tElement = from_coordinates<ElementType>(
        coordinate_vector_to_array<kStkTopology, ElementTraits<kStkTopology>::kNumNodes>(aCoordinates));
    return tElement.centroid();
}

/// @brief Compute sensitivities of volume with respect to nodal coordinates for a single element with coordinates @a
/// aCoordinates as dictated by the Tesselation TraitType
template <stk::topology::topology_t kStkTopology>
auto volume_nodal_sensitivities_impl(const std::vector<common::Coordinate>& aCoordinates)
    -> std::array<common::Vector3, ElementTraits<kStkTopology>::kNumNodes>
{
    using ElementType = ElementTraits<kStkTopology>::ElementType;
    const ElementType tElement = from_coordinates<ElementType>(
        coordinate_vector_to_array<kStkTopology, ElementTraits<kStkTopology>::kNumNodes>(aCoordinates));
    return tElement.volumeVertexSensitivities();
}
}  // namespace detail

}  // namespace plato::third_party_integration::stk_io
#endif
