#ifndef PLATO_THIRDPARTYINTEGRATION_KRINO_LEVELSETPRIMITIVES
#define PLATO_THIRDPARTYINTEGRATION_KRINO_LEVELSETPRIMITIVES

#include <Akri_Composite_Surface.hpp>
#include <vector>

#include "plato/third_party_integration/common/Vector3.hpp"

namespace plato::third_party_integration::krino
{

struct Plane
{
    common::Vector3 mNormal{0.0, 0.0, 0.0};
    double mOffset = 0.0;
};

struct Sphere
{
    common::Coordinate mCenter{0.0, 0.0, 0.0};
    double mRadius = 0.0;
};

struct SphereCounts
{
    unsigned int mX;
    unsigned int mY;
    unsigned int mZ;
};

struct SphereLocatorData
{
    common::Coordinate mSpherePatternStart;
    SphereCounts mSphereCounts;
};

struct SpherePatternData
{
    common::Coordinate mBoundingBoxMinXYZ;
    common::Coordinate mBoundingBoxMaxXYZ;
    double mSphereRadius = 0.0;
    double mSphereSpacing = 1.0;
};

struct LevelSetPrimitives
{
    std::vector<Plane> mPlanes;
    std::vector<Sphere> mSpheres;
};

///@brief Initializes the krino composite surface so that primitive surface types can be added to it
///@pre krino has been initialized using 'initialize_environment_for_krino'
[[nodiscard]] auto make_krino_composite_surface() -> ::krino::Composite_Surface;

///@brief Take a vector of Spheres @a aSpheres and convert them to a krino native type and add them to the krino
/// composite surface @a aSurfaces.
///@pre krino has been initialized using 'initialize_environment_for_krino'
void append_spheres(::krino::Composite_Surface& aSurfaces, const std::vector<Sphere>& aSpheres);

///@brief Take a vector of Planes @a aPlanes and convert them to a krino native type and add them to the krino composite
/// surface @a aSurfaces.
///@pre krino has been initialized using 'initialize_environment_for_krino'
void append_planes(::krino::Composite_Surface& aSurfaces, const std::vector<Plane>& aPlanes);

}  // namespace plato::third_party_integration::krino

#endif
