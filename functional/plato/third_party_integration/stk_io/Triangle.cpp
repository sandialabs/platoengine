#include "plato/third_party_integration/stk_io/Triangle.hpp"

#include <array>
#include <cmath>

#include "plato/third_party_integration/common/Vector3.hpp"

namespace plato::third_party_integration::stk_io
{

auto Triangle::volume() const -> double
{
    const auto tAreaVector = areaVector();
    constexpr auto tAreaFactor = 0.5;
    return std::sqrt(dot(tAreaVector, tAreaVector)) * tAreaFactor;
}

auto Triangle::normal() const -> common::Vector3
{
    const common::Vector3 a = p1 - p0;
    const common::Vector3 b = p2 - p0;
    const common::Vector3 tACrossB = cross(a, b);

    return tACrossB * (1.0 / magnitude(tACrossB));
}

auto Triangle::centroid() const -> common::Coordinate
{
    constexpr auto tNumberOfPoints = 3.0;
    return (p0 + p1 + p2) / tNumberOfPoints;
}

auto Triangle::volumeVertexSensitivities() const -> VertexSensitivities
{
    const auto tAreaVector = areaVector();
    const auto tCoefficient = 0.5 / std::sqrt(dot(tAreaVector, tAreaVector));
    const auto tDvolDp0 = cross(p2 - p1, tAreaVector) * tCoefficient;
    const auto tDvolDp1 = cross(p0 - p2, tAreaVector) * tCoefficient;
    const auto tDvolDp2 = cross(p1 - p0, tAreaVector) * tCoefficient;
    return std::array{tDvolDp0, tDvolDp1, tDvolDp2};
}

auto Triangle::areaVector() const -> common::Vector3
{
    const auto a = p1 - p0;
    const auto b = p2 - p0;
    return cross(b, a);
}

}  // namespace plato::third_party_integration::stk_io
