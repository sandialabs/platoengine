#include "plato/filter/extension/kernel_filters/Wedge.hpp"

#include <cmath>
#include <numbers>

#include "plato/third_party_integration/common/Vector3.hpp"

namespace plato::filter::extension::kernel_filters
{

namespace
{
const auto kOrigin = third_party_integration::common::Coordinate{0, 0, 0};
const auto kXPlane = Plane(kOrigin, {1, 0, 0});

[[nodiscard]] auto unit_normal(const double aWedgeAngle) -> third_party_integration::common::UnitVector3
{
    const double tWedgeAngleInRadians = aWedgeAngle * std::numbers::pi / 180.0;
    return {std::cos(tWedgeAngleInRadians), std::sin(tWedgeAngleInRadians), 0};
}

[[nodiscard]] auto repeats(const double aWedgeAngle) -> unsigned int
{
    return static_cast<unsigned int>(360.0 / aWedgeAngle - 1);
}

}  // namespace

auto positive_quadrant_wedge(const double aWedgeAngle) -> Wedge
{
    assert(std::fmod(360, aWedgeAngle) == 0);
    return {kXPlane, Plane(kOrigin, unit_normal(aWedgeAngle)), repeats(aWedgeAngle)};
}

}  // namespace plato::filter::extension::kernel_filters
