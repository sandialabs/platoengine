#ifndef PLATO_THIRDPARTYINTEGRATION_STKIO_TETRAHEDRON
#define PLATO_THIRDPARTYINTEGRATION_STKIO_TETRAHEDRON

#include <array>

#include "plato/third_party_integration/common/Vector3.hpp"

namespace plato::third_party_integration::stk_io
{

struct Tetrahedron
{
    constexpr static auto mNumVertices = unsigned{4};
    using Indices = std::array<unsigned int, mNumVertices>;
    using VertexSensitivities = std::array<common::Vector3, mNumVertices>;

    common::Coordinate mP0;
    common::Coordinate mP1;
    common::Coordinate mP2;
    common::Coordinate mP3;

    [[nodiscard]] auto volume() const -> double;
    [[nodiscard]] auto centroid() const -> common::Coordinate;
    [[nodiscard]] auto volumeVertexSensitivities() const -> VertexSensitivities;
};

}  // namespace plato::third_party_integration::stk_io
#endif
