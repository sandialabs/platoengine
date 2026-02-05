#ifndef PLATO_THIRDPARTYINTEGRATION_STKIO_HEXAHEDRON
#define PLATO_THIRDPARTYINTEGRATION_STKIO_HEXAHEDRON

#include <array>

#include "plato/third_party_integration/common/Vector3.hpp"

namespace plato::third_party_integration::stk_io
{
/// @brief struct representing a primitive hexahedron for computing geometric quantities.
/// Vertices in the hexahedron are labeled based on their ordering relative to a unit hex aligned with the Cartesian
/// coordinate axes, e.g. p010 refers to the vertex at point (0,1,0).
struct Hexahedron
{
    constexpr static auto kNumVertices = unsigned{8};
    using Indices = std::array<unsigned int, kNumVertices>;
    using VertexSensitivities = std::array<common::Vector3, kNumVertices>;

    common::Coordinate p000;
    common::Coordinate p100;
    common::Coordinate p110;
    common::Coordinate p010;
    common::Coordinate p001;
    common::Coordinate p101;
    common::Coordinate p111;
    common::Coordinate p011;

    [[nodiscard]] double volume() const;
    [[nodiscard]] common::Coordinate centroid() const;
    [[nodiscard]] auto volumeVertexSensitivities() const -> VertexSensitivities;
};
}  // namespace plato::third_party_integration::stk_io

#endif
