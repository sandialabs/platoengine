#include "plato/third_party_integration/krino/SphereFactory.hpp"

#include <cstddef>

#include "plato/utilities/MultidimensionalRange.hpp"
#include "plato/utilities/NamedType.hpp"

namespace plato::third_party_integration::krino
{
std::vector<Sphere> generate_spheres(const SpherePatternData &aData)
{
    SphereLocatorData tLocatorData = detail::calculate_sphere_locator_data(aData);

    std::vector<double> tXValues = detail::create_sphere_spacing_vector(
        tLocatorData.mSphereCounts.mX, detail::SphereStart{tLocatorData.mSpherePatternStart.x},
        detail::SphereStep{aData.mSphereSpacing});
    std::vector<double> tYValues = detail::create_sphere_spacing_vector(
        tLocatorData.mSphereCounts.mY, detail::SphereStart{tLocatorData.mSpherePatternStart.y},
        detail::SphereStep{aData.mSphereSpacing});
    std::vector<double> tZValues = detail::create_sphere_spacing_vector(
        tLocatorData.mSphereCounts.mZ, detail::SphereStart{tLocatorData.mSpherePatternStart.z},
        detail::SphereStep{aData.mSphereSpacing});

    // Loop to create 3D array of spheres
    std::vector<Sphere> tSpheres;
    tSpheres.reserve(static_cast<std::vector<Sphere>::size_type>(tLocatorData.mSphereCounts.mX) *
                     tLocatorData.mSphereCounts.mY * tLocatorData.mSphereCounts.mZ);

    for (const auto &[tXIndex, tYIndex, tZIndex] :
         utilities::MultidimensionalRange{tXValues.size(), tYValues.size(), tZValues.size()})
    {
        tSpheres.push_back(Sphere{{tXValues[tXIndex], tYValues[tYIndex], tZValues[tZIndex]}, aData.mSphereRadius});
    }

    return tSpheres;
}

namespace detail
{
namespace
{
using IntegerCoordinate = utilities::NamedType<common::Coordinate, struct IntegerCoordinateTag>;
[[nodiscard]] auto make_sphere_locator_data(const common::Coordinate &aStartLocations,
                                            const IntegerCoordinate &aNumSpheres) -> SphereLocatorData
{
    return SphereLocatorData{
        aStartLocations,
        {static_cast<unsigned int>(aNumSpheres.mValue.x), static_cast<unsigned int>(aNumSpheres.mValue.y),
         static_cast<unsigned int>(aNumSpheres.mValue.z)}};
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

std::vector<double> create_sphere_spacing_vector(const unsigned int aNumValues,
                                                 const SphereStart aStart,
                                                 const SphereStep aStep)
{
    auto tStart = aStart.mValue;
    std::vector<double> tValues(aNumValues);
    std::generate(tValues.begin(), tValues.end(),
                  [&tStart, &aStep]
                  {
                      double tRet = tStart;
                      tStart += aStep.mValue;
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
}  // namespace detail

}  // namespace plato::third_party_integration::krino
