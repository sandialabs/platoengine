#include "plato/third_party_integration/krino/SensitivityTriangle.hpp"

#include <cmath>

#include "plato/third_party_integration/common/Vector3.hpp"

namespace plato::third_party_integration::krino
{

auto SensitivityTriangle::normal() const -> common::UnitVector3
{
    const common::Vector3 a = mNodes[1].second - mNodes[0].second;
    const common::Vector3 b = mNodes[2].second - mNodes[0].second;
    const common::Vector3 tACrossB = cross(a, b);

    return tACrossB * (1.0 / magnitude(tACrossB));
}

auto SensitivityTriangle::area() const -> double
{
    const common::Vector3 a = mNodes[1].second - mNodes[0].second;
    const common::Vector3 b = mNodes[2].second - mNodes[0].second;
    const common::Vector3 tACrossB = cross(a, b);
    constexpr auto tTriangleAreaFactor = 0.5;

    return magnitude(tACrossB) * tTriangleAreaFactor;
}

}  // namespace plato::third_party_integration::krino
