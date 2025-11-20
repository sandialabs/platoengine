#ifndef PLATO_THIRDPARTYINTEGRATION_STKIO_TRIANGLE
#define PLATO_THIRDPARTYINTEGRATION_STKIO_TRIANGLE

#include <array>

#include "plato/third_party_integration/common/Vector3.hpp"

namespace plato::third_party_integration::stk_io
{
struct Triangle
{
    common::Coordinate p0;
    common::Coordinate p1;
    common::Coordinate p2;

    [[nodiscard]] double volume() const;
    [[nodiscard]] common::Vector3 normal() const;
    [[nodiscard]] common::Coordinate centroid() const;
};

}  // namespace plato::third_party_integration::stk_io
#endif
