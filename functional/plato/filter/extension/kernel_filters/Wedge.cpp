#include "plato/filter/extension/kernel_filters/Wedge.hpp"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <numbers>
#include <vector>

#include "plato/filter/extension/kernel_filters/Plane.hpp"
#include "plato/third_party_integration/common/Vector3.hpp"

namespace plato::filter::extension::kernel_filters
{

namespace
{
const auto kOrigin = third_party_integration::common::Coordinate{0, 0, 0};
const auto kYPlane = Plane(kOrigin, {0, 1, 0});
constexpr auto kDegreesToRadians = std::numbers::pi / 180.0;
constexpr auto kDegreesPerCircle = 360.0;

[[nodiscard]] auto unit_normal(const double aWedgeAngle) -> third_party_integration::common::UnitVector3
{
    const double tWedgeAngleInRadians = aWedgeAngle * kDegreesToRadians;
    return {-std::sin(tWedgeAngleInRadians), std::cos(tWedgeAngleInRadians), 0};
}

[[nodiscard]] auto repeats(const double aWedgeAngle) -> unsigned int
{
    return static_cast<unsigned int>(kDegreesPerCircle / aWedgeAngle - 1);
}

}  // namespace

auto positive_quadrant_wedge(const double aWedgeAngle) -> Wedge
{
    assert(std::fmod(kDegreesPerCircle, aWedgeAngle) == 0);
    return {kYPlane, Plane(kOrigin, unit_normal(aWedgeAngle)), repeats(aWedgeAngle)};
}

auto wedge_to_planes(const Wedge& aWedge) -> std::vector<Plane>
{
    assert(aWedge.mRepeats > 0);
    auto tPlanes = std::vector<Plane>(aWedge.mRepeats + 1);
    tPlanes[0] = aWedge.mFirstPlane;
    tPlanes[1] = aWedge.mSecondPlane;

    std::transform(tPlanes.begin() + 1, tPlanes.end() - 1, tPlanes.begin(), tPlanes.begin() + 2,
                   [](const Plane& aPlaneMinusOne, const Plane& aPlaneMinusTwo)
                   { return aPlaneMinusOne.reflect(aPlaneMinusTwo); });

    tPlanes.erase(tPlanes.begin());
    return tPlanes;
}

}  // namespace plato::filter::extension::kernel_filters
