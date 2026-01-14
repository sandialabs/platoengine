#include "plato/filter/extension/kernel_filters/Plane.hpp"

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
    const auto tNewPoint = tpic::Vector3{aCoordinate.x, aCoordinate.y, aCoordinate.z} -
                           static_cast<tpic::Vector3>(mUnitNormal) * tDistanceFromCoordinateToPlane * 2.0;

    return tpic::Coordinate{tNewPoint.x, tNewPoint.y, tNewPoint.z};
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
    const auto tNormal = tpic::Coordinate{tUnitNormal.x, tUnitNormal.y, tUnitNormal.z};
    const auto tReflectedNormal = this->reflect(tNormal);
    const auto tReflectedCenter = this->reflect(aPlane.mCenter);

    return Plane(tReflectedCenter, tpic::UnitVector3{tReflectedNormal.x, tReflectedNormal.y, tReflectedNormal.z});
}

}  // namespace plato::filter::extension::kernel_filters
