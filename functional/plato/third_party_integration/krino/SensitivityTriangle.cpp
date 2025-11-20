#include "plato/third_party_integration/krino/SensitivityTriangle.hpp"

#include <cmath>

namespace plato::third_party_integration::krino
{

common::UnitVector3 SensitivityTriangle::normal() const
{
    const common::Vector3 a = mNode2.mCoordinates - mNode1.mCoordinates;
    const common::Vector3 b = mNode3.mCoordinates - mNode1.mCoordinates;
    const common::Vector3 tACrossB = cross(a, b);

    return tACrossB * (1.0 / magnitude(tACrossB));
}

double SensitivityTriangle::area() const
{
    const common::Vector3 a = mNode2.mCoordinates - mNode1.mCoordinates;
    const common::Vector3 b = mNode3.mCoordinates - mNode1.mCoordinates;
    const common::Vector3 tACrossB = cross(a, b);

    return 0.5 * magnitude(tACrossB);
}

}  // namespace plato::third_party_integration::krino
