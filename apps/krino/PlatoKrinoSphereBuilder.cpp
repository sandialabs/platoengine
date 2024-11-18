#include "PlatoKrinoSphereBuilder.hpp"

#include <cstddef>

#include "plato/utilities/NamedType.hpp"

namespace Plato
{

namespace
{
namespace ptpic = plato::third_party_integration::common;

using IntegerCoordinate = plato::utilities::NamedType<ptpic::Coordinate, struct IntegerCoordinateTag>;
[[nodiscard]] auto make_sphere_locator_data(const ptpic::Coordinate &aStartLocations,
                                            const IntegerCoordinate &aNumSpheres) -> SphereLocatorData
{
    return SphereLocatorData{
        aStartLocations,
        {static_cast<unsigned int>(aNumSpheres.mValue.x), static_cast<unsigned int>(aNumSpheres.mValue.y),
         static_cast<unsigned int>(aNumSpheres.mValue.z)}};
}
}  // namespace

ptpic::Coordinate calculate_num_spheres_in_each_direction(const SpherePatternData &aData)
{
    constexpr double tNumDirectionsFromCenterSphere = 2.0;
    constexpr double tCenterSphere = 1.0;
    const ptpic::Coordinate tBBoxHalfExtents{(aData.mBoundingBoxMaxXYZ + aData.mBoundingBoxMinXYZ * (-1.0)) / 2.0};
    const ptpic::Coordinate tBBoxHalfExtentsOverSpacing{tBBoxHalfExtents / aData.mSphereSpacing};
    const ptpic::Coordinate tRemainder{tBBoxHalfExtentsOverSpacing + floor(tBBoxHalfExtentsOverSpacing) * (-1.0)};
    return ptpic::Coordinate{
        (floor(tBBoxHalfExtentsOverSpacing) +
         floor(tRemainder * aData.mSphereSpacing / (aData.mSphereSpacing - aData.mSphereRadius))) *
            tNumDirectionsFromCenterSphere +
        tCenterSphere};
}

ptpic::Coordinate calculate_sphere_pattern_start(const ptpic::Coordinate &aNumSpheres, const SpherePatternData &aData)
{
    constexpr double tNumDirectionsFromCenterSphere = 2.0;
    constexpr double tCenterSphere = 1.0;
    const ptpic::Coordinate tBBoxCenter{(aData.mBoundingBoxMaxXYZ + aData.mBoundingBoxMinXYZ) / 2.0};
    return ptpic::Coordinate{tBBoxCenter + ((aNumSpheres + (-tCenterSphere)) / tNumDirectionsFromCenterSphere) *
                                                aData.mSphereSpacing * (-1.0)};
}

std::vector<double> calculate_sphere_center_coords(const unsigned int aNumValues,
                                                   NamedSphereCenterCoord aStart,
                                                   const double aStep)
{
    std::vector<double> tValues(aNumValues);
    std::generate(tValues.begin(), tValues.end(),
                  [&aStart, &aStep]
                  {
                      double tRet = aStart.mValue;
                      aStart.mValue += aStep;
                      return tRet;
                  });
    return tValues;
}

SphereLocatorData calculate_sphere_locator_data(const SpherePatternData &aData)
{
    const auto tNumSpheres = calculate_num_spheres_in_each_direction(aData);
    const auto tSpherePatternStart = calculate_sphere_pattern_start(tNumSpheres, aData);
    return make_sphere_locator_data(tSpherePatternStart, IntegerCoordinate{tNumSpheres});
}

std::vector<Sphere> generate_spheres(const SpherePatternData &aData)
{
    SphereLocatorData tLocatorData = calculate_sphere_locator_data(aData);

    std::vector<double> tXValues = calculate_sphere_center_coords(
        tLocatorData.mSphereCounts.mX, NamedSphereCenterCoord{tLocatorData.mSpherePatternStart.x},
        aData.mSphereSpacing);
    std::vector<double> tYValues = calculate_sphere_center_coords(
        tLocatorData.mSphereCounts.mY, NamedSphereCenterCoord{tLocatorData.mSpherePatternStart.y},
        aData.mSphereSpacing);
    std::vector<double> tZValues = calculate_sphere_center_coords(
        tLocatorData.mSphereCounts.mZ, NamedSphereCenterCoord{tLocatorData.mSpherePatternStart.z},
        aData.mSphereSpacing);

    // Loop to create 3D array of spheres
    std::vector<Sphere> tSpheres;
    tSpheres.reserve(static_cast<std::vector<Sphere>::size_type>(tLocatorData.mSphereCounts.mX) *
                     tLocatorData.mSphereCounts.mY * tLocatorData.mSphereCounts.mZ);

    for (auto tCurX : tXValues)
    {
        for (auto tCurY : tYValues)
        {
            for (auto tCurZ : tZValues)
            {
                tSpheres.push_back(Sphere{{tCurX, tCurY, tCurZ}, aData.mSphereRadius});
            }
        }
    }
    return tSpheres;
}

}  // namespace Plato
