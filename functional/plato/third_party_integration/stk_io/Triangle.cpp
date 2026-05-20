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
    const common::Vector3 tEdgeVector1 = mP1 - mP0;
    const common::Vector3 tEdgeVector2 = mP2 - mP0;
    const common::Vector3 tACrossB = cross(tEdgeVector1, tEdgeVector2);

    return tACrossB * (1.0 / magnitude(tACrossB));
}

auto Triangle::centroid() const -> common::Coordinate
{
    constexpr auto tNumberOfPoints = 3.0;
    return (mP0 + mP1 + mP2) / tNumberOfPoints;
}

auto Triangle::volumeVertexSensitivities() const -> VertexSensitivities
{
    const auto tAreaVector = areaVector();
    const auto tCoefficient = 0.5 / std::sqrt(dot(tAreaVector, tAreaVector));
    const auto tDvolDp0 = cross(mP2 - mP1, tAreaVector) * tCoefficient;
    const auto tDvolDp1 = cross(mP0 - mP2, tAreaVector) * tCoefficient;
    const auto tDvolDp2 = cross(mP1 - mP0, tAreaVector) * tCoefficient;
    return std::array{tDvolDp0, tDvolDp1, tDvolDp2};
}

auto Triangle::areaVector() const -> common::Vector3
{
    const auto tEdgeVector1 = mP1 - mP0;
    const auto tEdgeVector2 = mP2 - mP0;
    return cross(tEdgeVector2, tEdgeVector1);
}

}  // namespace plato::third_party_integration::stk_io
