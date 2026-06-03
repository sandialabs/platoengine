#ifndef PLATO_THIRDPARTYINTEGRATION_STKIO_QUADRILATERAL
#define PLATO_THIRDPARTYINTEGRATION_STKIO_QUADRILATERAL

#include <array>

#include "plato/third_party_integration/common/Vector3.hpp"
#include "plato/utilities/NamedType.hpp"

namespace plato::third_party_integration::stk_io
{
/// @brief struct representing a primitive quadrilateral for computing geometric quantities.
/// Vertices in the quadrilateral are labeled based on their ordering relative to a unit quad in a Cartesian plane
/// aligned with the coordinate axes, e.g. mP01 refers to the vertex at point (0,1) in the plane.

/// @note The implementation currently uses a third order quadrature rule for integration. This will be exact for planar
/// quadrilaterals but there will be errors for quadrilaterals with non-constant metric.
struct Quadrilateral
{
    constexpr static auto mNumVertices = unsigned{4};
    using Indices = std::array<unsigned int, mNumVertices>;
    using VertexSensitivities =
        utilities::NamedType<std::array<common::Vector3, mNumVertices>, struct VertexSensitivitiesTag>;

   public:
    [[nodiscard]] auto volume() const -> double;
    [[nodiscard]] auto centroid() const -> common::Coordinate;
    [[nodiscard]] auto volumeVertexSensitivities() const -> std::array<common::Vector3, mNumVertices>;

   private:
    [[nodiscard]] auto etaTangentVector(double aXiCoordinate) const -> common::Vector3;
    [[nodiscard]] auto xiTangentVector(double aEtaCoordinate) const -> common::Vector3;

   public:
    common::Coordinate mP00;
    common::Coordinate mP10;
    common::Coordinate mP11;
    common::Coordinate mP01;
};

[[nodiscard]] auto operator+(const Quadrilateral::VertexSensitivities& aLeftHandSide,
                             const Quadrilateral::VertexSensitivities& aRightHandSide)
    -> Quadrilateral::VertexSensitivities;

[[nodiscard]] auto operator*(const Quadrilateral::VertexSensitivities& aSensitivities,
                             double aScalar) -> Quadrilateral::VertexSensitivities;
}  // namespace plato::third_party_integration::stk_io

#endif
