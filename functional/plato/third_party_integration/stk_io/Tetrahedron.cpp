#include "plato/third_party_integration/stk_io/Tetrahedron.hpp"

#include <array>

#include "plato/third_party_integration/common/Vector3.hpp"

namespace plato::third_party_integration::stk_io
{

auto Tetrahedron::volume() const -> double
{
    const common::Vector3 tEdgeVector1 = mP1 - mP0;
    const common::Vector3 tEdgeVector2 = mP2 - mP0;
    const common::Vector3 tEdgeVector3 = mP3 - mP0;
    constexpr auto tVolumeFactor = 1.0 / 6.0;
    return dot(tEdgeVector1, cross(tEdgeVector2, tEdgeVector3)) * tVolumeFactor;
}

auto Tetrahedron::centroid() const -> common::Coordinate { return (mP0 + mP1 + mP2 + mP3) / 4; }

auto Tetrahedron::volumeVertexSensitivities() const -> VertexSensitivities
{
    const auto tCross32 = cross(mP3 - mP0, mP2 - mP0);
    const auto tCross31 = cross(mP3 - mP0, mP0 - mP1);
    const auto tCross21 = cross(mP2 - mP0, mP1 - mP0);

    const auto tDvolDp0 = tCross32 + tCross31 + tCross21;
    const auto tDvolDp1 = tCross32 * -1.0;
    const auto tDvolDp2 = tCross31 * -1.0;
    const auto tDvolDp3 = tCross21 * -1.0;
    constexpr auto tVolumeFactor = 1.0 / 6.0;
    return std::array{tDvolDp0 * tVolumeFactor, tDvolDp1 * tVolumeFactor, tDvolDp2 * tVolumeFactor,
                      tDvolDp3 * tVolumeFactor};
}

}  // namespace plato::third_party_integration::stk_io
