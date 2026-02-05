#ifndef PLATO_THIRDPARTYINTEGRATION_STKIO_QUADRILATERAL
#define PLATO_THIRDPARTYINTEGRATION_STKIO_QUADRILATERAL

#include <array>

#include "plato/third_party_integration/common/Vector3.hpp"
#include "plato/utilities/NamedType.hpp"

namespace plato::third_party_integration::stk_io
{
/// @brief struct representing a primitive quadrilateral for computing geometric quantities.
/// Vertices in the quadrilateral are labeled based on their ordering relative to a unit quad in a Cartesian plane
/// aligned with the coordinate axes, e.g. p01 refers to the vertex at point (0,1) in the plane.

/// @note The implementation currently uses a third order quadrature rule for integration. This will be exact for planar
/// quadrilaterals but there will be errors for quadrilaterals with non-constant metric.
struct Quadrilateral
{
    constexpr static auto kNumVertices = unsigned{4};
    using Indices = std::array<unsigned int, kNumVertices>;
    using VertexSensitivities =
        utilities::NamedType<std::array<common::Vector3, kNumVertices>, struct VertexSensitivitiesTag>;

   public:
    [[nodiscard]] double volume() const;
    [[nodiscard]] common::Coordinate centroid() const;
    [[nodiscard]] auto volumeVertexSensitivities() const -> std::array<common::Vector3, kNumVertices>;

   private:
    [[nodiscard]] common::Vector3 etaTangentVector(const double aCoordinate) const;
    [[nodiscard]] common::Vector3 xiTangentVector(const double aCoordinate) const;

   public:
    common::Coordinate p00;
    common::Coordinate p10;
    common::Coordinate p11;
    common::Coordinate p01;
};

[[nodiscard]] Quadrilateral::VertexSensitivities operator+(const Quadrilateral::VertexSensitivities& aLeftHandSide,
                                                           const Quadrilateral::VertexSensitivities& aRightHandSide);

[[nodiscard]] Quadrilateral::VertexSensitivities operator*(const Quadrilateral::VertexSensitivities& aSensitivities,
                                                           const double aScalar);
}  // namespace plato::third_party_integration::stk_io

#endif
