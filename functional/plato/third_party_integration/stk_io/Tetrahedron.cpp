#include "plato/third_party_integration/stk_io/Tetrahedron.hpp"

#include <array>

#include "plato/third_party_integration/common/Vector3.hpp"

namespace plato::third_party_integration::stk_io
{

auto Tetrahedron::volume() const -> double
{
    const common::Vector3 a = p1 - p0;
    const common::Vector3 b = p2 - p0;
    const common::Vector3 c = p3 - p0;
    constexpr auto tVolumeFactor = 1.0 / 6.0;
    return dot(a, cross(b, c)) * tVolumeFactor;
}

auto Tetrahedron::centroid() const -> common::Coordinate { return (p0 + p1 + p2 + p3) / 4; }

auto Tetrahedron::volumeVertexSensitivities() const -> VertexSensitivities
{
    const auto tCross32 = cross(p3 - p0, p2 - p0);
    const auto tCross31 = cross(p3 - p0, p0 - p1);
    const auto tCross21 = cross(p2 - p0, p1 - p0);

    const auto tDvolDp0 = tCross32 + tCross31 + tCross21;
    const auto tDvolDp1 = tCross32 * -1.0;
    const auto tDvolDp2 = tCross31 * -1.0;
    const auto tDvolDp3 = tCross21 * -1.0;
    constexpr auto tVolumeFactor = 1.0 / 6.0;
    return std::array{tDvolDp0 * tVolumeFactor, tDvolDp1 * tVolumeFactor, tDvolDp2 * tVolumeFactor,
                      tDvolDp3 * tVolumeFactor};
}

}  // namespace plato::third_party_integration::stk_io
