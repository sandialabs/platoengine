#include "plato/third_party_integration/stk_io/Tetrahedron.hpp"

namespace plato::third_party_integration::stk_io
{

double Tetrahedron::volume() const
{
    const common::Vector3 a = p1 - p0;
    const common::Vector3 b = p2 - p0;
    const common::Vector3 c = p3 - p0;
    return dot(a, cross(b, c)) / 6.0;
}

common::Coordinate Tetrahedron::centroid() const { return (p0 + p1 + p2 + p3) / 4; }

auto Tetrahedron::volumeVertexSensitivities() const -> VertexSensitivities
{
    const auto tCross32 = cross(p3 - p0, p2 - p0);
    const auto tCross31 = cross(p3 - p0, p0 - p1);
    const auto tCross21 = cross(p2 - p0, p1 - p0);

    const auto tDvolDp0 = tCross32 + tCross31 + tCross21;
    const auto tDvolDp1 = tCross32 * -1.0;
    const auto tDvolDp2 = tCross31 * -1.0;
    const auto tDvolDp3 = tCross21 * -1.0;

    return std::array{tDvolDp0 * 1. / 6., tDvolDp1 * 1. / 6., tDvolDp2 * 1. / 6., tDvolDp3 * 1. / 6.};
}

}  // namespace plato::third_party_integration::stk_io
