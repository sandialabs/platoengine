#include "plato/third_party_integration/stk_io/Tetrahedron.hpp"

namespace plato::third_party_integration::stk_io
{

double Tetrahedron::volume() const
{
    const common::Vector3 a = p1 - p0;
    const common::Vector3 b = p2 - p0;
    const common::Vector3 c = p3 - p0;
    return dot(a, cross(b, c)) / 6.0;
}

common::Coordinate Tetrahedron::centroid() const { return (p0 + p1 + p2 + p3) / 4; }

}  // namespace plato::third_party_integration::stk_io
