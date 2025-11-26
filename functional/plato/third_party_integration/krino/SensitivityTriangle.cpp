#include "plato/third_party_integration/krino/SensitivityTriangle.hpp"

#include <cmath>

namespace plato::third_party_integration::krino
{

common::UnitVector3 SensitivityTriangle::normal() const
{
    const common::Vector3 a = mNodes[1].second - mNodes[0].second;
    const common::Vector3 b = mNodes[2].second - mNodes[0].second;
    const common::Vector3 tACrossB = cross(a, b);

    return tACrossB * (1.0 / magnitude(tACrossB));
}

double SensitivityTriangle::area() const
{
    const common::Vector3 a = mNodes[1].second - mNodes[0].second;
    const common::Vector3 b = mNodes[2].second - mNodes[0].second;
    const common::Vector3 tACrossB = cross(a, b);

    return 0.5 * magnitude(tACrossB);
}

}  // namespace plato::third_party_integration::krino
