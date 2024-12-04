#ifndef PLATO_THIRDPARTYINTEGRATION_KRINO_LEVELSETPRIMITIVES
#define PLATO_THIRDPARTYINTEGRATION_KRINO_LEVELSETPRIMITIVES

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

}  // namespace plato::third_party_integration::krino

#endif  // PLATO_THIRD_PARTY_INTEGRATION_KRINO_LEVELSET_PRIMITIVES
