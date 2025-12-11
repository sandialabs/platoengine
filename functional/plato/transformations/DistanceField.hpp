#ifndef PLATO_TRANSFORMATIONS_DISTANCEFIELD
#define PLATO_TRANSFORMATIONS_DISTANCEFIELD

#include "plato/analysis/AnalysisDomainMesh.hpp"
#include "plato/third_party_integration/common/Vector3.hpp"

namespace plato::transformations
{
/// @brief Representation of a plane using a normal and an offset from the origin (Hessian normal form).
/// @note `mOriginSignedDistance > 0` indicates that the half space formed by the plane and normal
/// contains the origin.
struct Plane
{
    double mOriginSignedDistance = 0.0;
    third_party_integration::common::Vector3 mNormal;
};

/// @brief Computes an element-wise distance field on @a aMesh, giving the signed distance from each element centroid to
/// the plane @a aPlane.
/// @note This transformation is affine in the nodal coordinates.
[[nodiscard]] auto element_centroid_distance_field(const analysis::AnalysisDomainMesh& aMesh,
                                                   const Plane& aPlane) -> analysis::AnalysisDomainMesh;

/// @brief Computes the multiplication of a row vector @a aRowVector with the Jacobian of the element centroid distance
/// field.
[[nodiscard]] auto row_vector_jacobian_multiplication_distance_field(const std::vector<double>& aRowVector,
                                                                     const analysis::AnalysisDomainMesh& aMesh,
                                                                     const Plane& aPlane) -> std::vector<double>;

/// @brief Computes the multiplication of a row vector @a aRowVector with the adjoint Jacobian of the element centroid
/// distance field.
[[nodiscard]] auto row_vector_adjoint_jacobian_multiplication_distance_field(const std::vector<double>& aRowVector,
                                                                             const analysis::AnalysisDomainMesh& aMesh,
                                                                             const Plane& aPlane)
    -> std::vector<double>;

namespace detail
{
/// @brief Computes the distance from a point @a aPoint to a plane @a aPlane.
[[nodiscard]] constexpr auto point_plane_signed_distance(
    const Plane& aPlane, const third_party_integration::common::Coordinate& aPoint) -> double;

constexpr auto point_plane_signed_distance(const Plane& aPlane,
                                           const third_party_integration::common::Coordinate& aPoint) -> double
{
    namespace tpic = third_party_integration::common;
    constexpr auto tOrigin = tpic::Coordinate{0.0, 0.0, 0.0};
    return tpic::dot(aPlane.mNormal, aPoint - tOrigin) / tpic::magnitude(aPlane.mNormal) + aPlane.mOriginSignedDistance;
}
}  // namespace detail
}  // namespace plato::transformations

#endif
