#include "plato/utilities/Tetrahedron.hpp"

namespace plato::utilities
{

double Tetrahedron::volume() const
{
    const Vector3 a = p1 - p0;
    const Vector3 b = p2 - p0;
    const Vector3 c = p3 - p0;
    return dot(a, cross(b, c)) / 6.0;
}

}  // namespace plato::utilities
