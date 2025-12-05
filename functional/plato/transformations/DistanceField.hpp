#ifndef PLATO_TRANSFORMATIONS_DISTANCEFIELD
#define PLATO_TRANSFORMATIONS_DISTANCEFIELD

#include "plato/third_party_integration/common/Vector3.hpp"

namespace plato::transformations
{
/// @brief Representation of a plane using a normal and an offset from the origin (Hessian normal form).
/// @note The sign of `mOriginSignedDistance` is positive if the half space formed by the plane and normal contains the
/// origin, and it is negative otherwise.
struct Plane
{
    double mOriginSignedDistance = 0.0;
    third_party_integration::common::Vector3 mNormal;
};

/// @brief Computes the distance from a point @a aPoint to a plane @a aPlane.
[[nodiscard]] constexpr auto point_plane_signed_distance(
    const Plane aPlane, const third_party_integration::common::Coordinate& aPoint) -> double;

constexpr auto point_plane_signed_distance(const Plane aPlane,
                                           const third_party_integration::common::Coordinate& aPoint) -> double
{
    namespace tpic = third_party_integration::common;
    constexpr auto tOrigin = tpic::Coordinate{0.0, 0.0, 0.0};
    return tpic::dot(aPlane.mNormal, aPoint - tOrigin) / tpic::magnitude(aPlane.mNormal) + aPlane.mOriginSignedDistance;
}

}  // namespace plato::transformations

#endif
