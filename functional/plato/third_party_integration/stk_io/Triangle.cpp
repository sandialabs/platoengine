#include "plato/third_party_integration/stk_io/Triangle.hpp"

#include <cmath>

namespace plato::third_party_integration::stk_io
{

double Triangle::volume() const
{
    const common::Vector3 a = p1 - p0;
    const common::Vector3 b = p2 - p0;
    const common::Vector3 tBCrossA = cross(b, a);

    return std::sqrt(dot(tBCrossA, tBCrossA)) * 0.5;
}

common::Vector3 Triangle::normal() const
{
    const common::Vector3 a = p1 - p0;
    const common::Vector3 b = p2 - p0;
    const common::Vector3 tACrossB = cross(a, b);

    return tACrossB * (1.0 / magnitude(tACrossB));
}

common::Coordinate Triangle::centroid() const { return (p0 + p1 + p2) / 3; }

}  // namespace plato::third_party_integration::stk_io
