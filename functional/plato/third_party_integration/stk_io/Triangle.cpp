#include "plato/third_party_integration/stk_io/Triangle.hpp"

#include <cmath>

namespace plato::third_party_integration::stk_io
{

double Triangle::volume() const
{
    const auto tAreaVector = areaVector();
    return std::sqrt(dot(tAreaVector, tAreaVector)) * 0.5;
}

common::Vector3 Triangle::normal() const
{
    const common::Vector3 a = p1 - p0;
    const common::Vector3 b = p2 - p0;
    const common::Vector3 tACrossB = cross(a, b);

    return tACrossB * (1.0 / magnitude(tACrossB));
}

common::Coordinate Triangle::centroid() const { return (p0 + p1 + p2) / 3; }

auto Triangle::volumeVertexSensitivities() const -> VertexSensitivities
{
    const auto tAreaVector = areaVector();
    const auto tCoefficient = 0.5 / std::sqrt(dot(tAreaVector, tAreaVector));
    const auto tDvolDp0 = cross(p2 - p1, tAreaVector) * tCoefficient;
    const auto tDvolDp1 = cross(p0 - p2, tAreaVector) * tCoefficient;
    const auto tDvolDp2 = cross(p1 - p0, tAreaVector) * tCoefficient;
    return std::array{tDvolDp0, tDvolDp1, tDvolDp2};
}

common::Vector3 Triangle::areaVector() const
{
    const auto a = p1 - p0;
    const auto b = p2 - p0;
    return cross(b, a);
}

}  // namespace plato::third_party_integration::stk_io
