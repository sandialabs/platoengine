#include "plato/third_party_integration/krino/SphereBuilder.hpp"

#include "plato/utilities/NamedType.hpp"

namespace plato::third_party_integration::krino
{

namespace
{
using IntegerCoordinate = utilities::NamedType<common::Coordinate, struct IntegerCoordinateTag>;
[[nodiscard]] auto make_sphere_locator_data(const common::Coordinate &aStartLocations,
                                            const IntegerCoordinate &aNumSpheres) -> SphereLocatorData
{
    return SphereLocatorData{aStartLocations,
                             {static_cast<int>(aNumSpheres.mValue.x), static_cast<int>(aNumSpheres.mValue.y),
                              static_cast<int>(aNumSpheres.mValue.z)}};
}
}  // namespace

common::Coordinate calculate_num_spheres_in_each_direction(const SpherePatternData &aData)
{
    constexpr double tNumDirectionsFromCenterSphere = 2.0;
    constexpr double tCenterSphere = 1.0;
    const common::Coordinate tBBoxHalfExtents{(aData.mBoundingBoxMaxXYZ + aData.mBoundingBoxMinXYZ * (-1.0)) / 2.0};
    const common::Coordinate tBBoxHalfExtentsOverSpacing{tBBoxHalfExtents / aData.mSphereSpacing};
    const common::Coordinate tRemainder{tBBoxHalfExtentsOverSpacing + floor(tBBoxHalfExtentsOverSpacing) * (-1.0)};
    return common::Coordinate{
        (floor(tBBoxHalfExtentsOverSpacing) +
         floor(tRemainder * aData.mSphereSpacing / (aData.mSphereSpacing - aData.mSphereRadius))) *
            tNumDirectionsFromCenterSphere +
        tCenterSphere};
}

common::Coordinate calculate_sphere_pattern_start(const common::Coordinate &aNumSpheres, const SpherePatternData &aData)
{
    constexpr double tNumDirectionsFromCenterSphere = 2.0;
    constexpr double tCenterSphere = 1.0;
    const common::Coordinate tBBoxCenter{(aData.mBoundingBoxMaxXYZ + aData.mBoundingBoxMinXYZ) / 2.0};
    return common::Coordinate{tBBoxCenter + ((aNumSpheres + (-tCenterSphere)) / tNumDirectionsFromCenterSphere) *
                                                aData.mSphereSpacing * (-1.0)};
}

std::vector<double> calculate_sphere_center_coords(int aNumValues, double aStart, double aStep)
{
    std::vector<double> tValues(aNumValues);
    std::generate(tValues.begin(), tValues.end(),
                  [&aStart, &aStep]
                  {
                      double tRet = aStart;
                      aStart += aStep;
                      return tRet;
                  });
    return tValues;
}

SphereLocatorData calculate_sphere_locator_data(const SpherePatternData &aData)
{
    const common::Coordinate tNumSpheres = calculate_num_spheres_in_each_direction(aData);
    const common::Coordinate tSpherePatternStart = calculate_sphere_pattern_start(tNumSpheres, aData);
    return make_sphere_locator_data(tSpherePatternStart, IntegerCoordinate{tNumSpheres});
}

std::vector<Sphere> generate_spheres(const SpherePatternData &aData)
{
    SphereLocatorData tLocatorData = calculate_sphere_locator_data(aData);

    std::vector<double> tXValues = calculate_sphere_center_coords(
        tLocatorData.mSphereCounts.mX, tLocatorData.mSpherePatternStart.x, aData.mSphereSpacing);
    std::vector<double> tYValues = calculate_sphere_center_coords(
        tLocatorData.mSphereCounts.mY, tLocatorData.mSpherePatternStart.y, aData.mSphereSpacing);
    std::vector<double> tZValues = calculate_sphere_center_coords(
        tLocatorData.mSphereCounts.mZ, tLocatorData.mSpherePatternStart.z, aData.mSphereSpacing);

    // Loop to create 3D array of spheres
    std::vector<Sphere> tSpheres;
    tSpheres.reserve(tLocatorData.mSphereCounts.mX * tLocatorData.mSphereCounts.mY * tLocatorData.mSphereCounts.mZ);

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

}  // namespace plato::third_party_integration::krino
