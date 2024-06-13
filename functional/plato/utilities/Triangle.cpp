#include "plato/utilities/Triangle.hpp"

#include <cmath>

namespace plato::utilities
{
double Triangle::volume() const
{
    const Vector3 a = p1 - p0;
    const Vector3 b = p2 - p0;
    const Vector3 tBCrossA = cross(b, a);

    return std::sqrt(dot(tBCrossA, tBCrossA)) * 0.5;
}
}  // namespace plato::utilities
