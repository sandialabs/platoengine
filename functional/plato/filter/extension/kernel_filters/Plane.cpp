#include "plato/filter/extension/kernel_filters/Plane.hpp"

#include <cassert>

#include "plato/third_party_integration/common/Vector3.hpp"
#include "plato/third_party_integration/stk_search/Utilities.hpp"

namespace plato::filter::extension::kernel_filters
{

auto Plane::reflect(const third_party_integration::common::Coordinate& aCoordinate) const
    -> third_party_integration::common::Coordinate
{
    namespace tpic = third_party_integration::common;
    assert(tpic::magnitude(static_cast<tpic::Vector3>(mUnitNormal)) == 1);
    const auto tDistanceFromCoordinateToPlane =
        tpic::dot((aCoordinate - mCenter), static_cast<tpic::Vector3>(mUnitNormal));
    const auto tNewPoint = tpic::Vector3{aCoordinate.mX, aCoordinate.mY, aCoordinate.mZ} -
                           static_cast<tpic::Vector3>(mUnitNormal) * tDistanceFromCoordinateToPlane * 2.0;

    return tpic::Coordinate{tNewPoint.mX, tNewPoint.mY, tNewPoint.mZ};
}

auto Plane::reflect(const third_party_integration::stk_search::SearchPointWithIdentifier& aSearchPointWithIdentifier)
    const -> third_party_integration::stk_search::SearchPointWithIdentifier
{
    namespace tpi = third_party_integration;
    const auto tIdentifier = aSearchPointWithIdentifier.second;
    const auto tReflectedPoint = reflect(tpi::stk_search::convert_search_point(aSearchPointWithIdentifier.first));
    return tpi::stk_search::SearchPointWithIdentifier{tpi::stk_search::convert_coordinate(tReflectedPoint),
                                                      tIdentifier};
}

auto Plane::reflect(const Plane& aPlane) const -> Plane
{
    namespace tpic = third_party_integration::common;
    const auto& tUnitNormal = static_cast<tpic::Vector3>(aPlane.mUnitNormal);
    const auto tNormal = tpic::Coordinate{tUnitNormal.mX, tUnitNormal.mY, tUnitNormal.mZ};
    const auto tReflectedNormal = this->reflect(tNormal);
    const auto tReflectedCenter = this->reflect(aPlane.mCenter);

    return Plane(tReflectedCenter, tpic::UnitVector3{tReflectedNormal.mX, tReflectedNormal.mY, tReflectedNormal.mZ});
}

}  // namespace plato::filter::extension::kernel_filters
