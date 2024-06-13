#ifndef PLATO_UTILITIES_TESSELATIONTRAITS
#define PLATO_UTILITIES_TESSELATIONTRAITS

#include <numeric>
#include <stk_topology/topology.hpp>
#include <vector>

#include "plato/utilities/Exception.hpp"
#include "plato/utilities/Tetrahedron.hpp"
#include "plato/utilities/Triangle.hpp"
#include "plato/utilities/Vector3.hpp"

namespace plato::utilities
{

/// @brief TesselationTraits is a templated struct that stores all the information required to tessalate an element
///
/// type from STK. kNumElements specifies the number of elements in the tesselation. ElementType specifies the type of
/// element to tesselate with, eg Triangle, Tetrahedron. kTesselationIndices is the array of indices that defines the
/// elements of the tesselation from the STK element relative indices.

template <stk::topology::topology_t STK_TOPOLOGY>
struct TesselationTraits
{
};

template <>
struct TesselationTraits<stk::topology::HEXAHEDRON_8>
{
    constexpr static auto kNumElements = 6;
    using ElementType = Tetrahedron;
    constexpr static std::array<ElementType::Indices, kNumElements> kTesselationIdices = {
        ElementType::Indices{4, 0, 3, 1}, ElementType::Indices{7, 4, 3, 1}, ElementType::Indices{7, 5, 4, 1},
        ElementType::Indices{7, 2, 5, 1}, ElementType::Indices{7, 3, 2, 1}, ElementType::Indices{7, 6, 5, 2}};
};

template <>
struct TesselationTraits<stk::topology::HEXAHEDRON_20> : public TesselationTraits<stk::topology::HEXAHEDRON_8>
{
};

template <>
struct TesselationTraits<stk::topology::TETRAHEDRON_4>
{
    constexpr static auto kNumElements = 1;
    using ElementType = Tetrahedron;
    constexpr static std::array<ElementType::Indices, kNumElements> kTesselationIdices = {
        ElementType::Indices{0, 1, 2, 3}};
};

template <>
struct TesselationTraits<stk::topology::TETRAHEDRON_10> : public TesselationTraits<stk::topology::TETRAHEDRON_4>
{
};

template <>
struct TesselationTraits<stk::topology::QUAD_4>
{
    constexpr static auto kNumElements = 2;
    using ElementType = Triangle;
    constexpr static std::array<ElementType::Indices, kNumElements> kTesselationIdices = {
        ElementType::Indices{0, 1, 2}, ElementType::Indices{0, 2, 3}};
};

template <>
struct TesselationTraits<stk::topology::SHELL_QUAD_4> : public TesselationTraits<stk::topology::QUAD_4>
{
};

template <>
struct TesselationTraits<stk::topology::QUAD_4_2D> : public TesselationTraits<stk::topology::QUAD_4>
{
};

template <>
struct TesselationTraits<stk::topology::TRIANGLE_3>
{
    constexpr static auto kNumElements = 1;
    using ElementType = Triangle;
    constexpr static std::array<ElementType::Indices, kNumElements> kTesselationIdices = {
        ElementType::Indices{0, 1, 2}};
};

template <>
struct TesselationTraits<stk::topology::TRIANGLE_3_2D> : public TesselationTraits<stk::topology::TRIANGLE_3>
{
};

template <>
struct TesselationTraits<stk::topology::SHELL_TRIANGLE_3> : public TesselationTraits<stk::topology::TRIANGLE_3>
{
};

namespace detail
{

/// @brief Create a ReturnObject from the coordinates specified in @a aCoordinates according to the tesselation indices
/// @a aIndices
template <typename ReturnObject>
ReturnObject from_coordinates(const std::array<unsigned int, ReturnObject::kNumVertices>& aIndices,
                              const std::vector<Coordinate>& aCoordinates);

/// @brief Determine the volume of a single element with coordinates @a aCoordinates as dictated by the Tesselation
/// TraitType
template <stk::topology::topology_t STK_TOPOLOGY>
double volume_impl(const std::vector<Coordinate>& aCoordinates)
{
    using TraitType = TesselationTraits<STK_TOPOLOGY>;
    std::array<typename TraitType::ElementType, TraitType::kNumElements> tTesselation;
    std::transform(TraitType::kTesselationIdices.cbegin(), TraitType::kTesselationIdices.cend(), tTesselation.begin(),
                   [&aCoordinates](const std::array<unsigned int, TraitType::ElementType::kNumVertices>& tIndices)
                   { return from_coordinates<typename TraitType::ElementType>(tIndices, aCoordinates); });

    return std::accumulate(tTesselation.cbegin(), tTesselation.cend(), 0.0,
                           [](const double aResult, const typename TraitType::ElementType& aReturnObject)
                           { return aResult + aReturnObject.volume(); });
}

}  // namespace detail

}  // namespace plato::utilities
#endif
