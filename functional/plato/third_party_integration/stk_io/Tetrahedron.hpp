#ifndef PLATO_THIRDPARTYINTEGRATION_STKIO_TETRAHEDRON
#define PLATO_THIRDPARTYINTEGRATION_STKIO_TETRAHEDRON

#include <array>

#include "plato/third_party_integration/common/Vector3.hpp"
namespace plato::third_party_integration::stk_io
{

struct Tetrahedron
{
    common::Coordinate p0;
    common::Coordinate p1;
    common::Coordinate p2;
    common::Coordinate p3;

    [[nodiscard]] double volume() const;
    [[nodiscard]] common::Coordinate centroid() const;

    constexpr static auto kNumVertices = unsigned{4};
    using Indices = std::array<unsigned int, kNumVertices>;
};

}  // namespace plato::third_party_integration::stk_io
#endif
