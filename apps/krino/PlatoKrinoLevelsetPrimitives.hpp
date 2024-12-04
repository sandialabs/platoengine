#ifndef PLATO_THIRD_PARTY_INTEGRATION_KRINO_LEVELSET_PRIMITIVES
#define PLATO_THIRD_PARTY_INTEGRATION_KRINO_LEVELSET_PRIMITIVES

#include <vector>

#include "plato/third_party_integration/common/Vector3.hpp"

namespace Plato
{

struct Plane
{
    plato::third_party_integration::common::Vector3 mNormal{0.0, 0.0, 0.0};
    double mOffset = 0.0;
};

struct Sphere
{
    plato::third_party_integration::common::Coordinate mCenter{0.0, 0.0, 0.0};
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
    plato::third_party_integration::common::Coordinate mSpherePatternStart;
    SphereCounts mSphereCounts;
};

struct SpherePatternData
{
    plato::third_party_integration::common::Coordinate mBoundingBoxMinXYZ;
    plato::third_party_integration::common::Coordinate mBoundingBoxMaxXYZ;
    double mSphereRadius = 0.0;
    double mSphereSpacing = 1.0;
};

struct LevelsetPrimitives
{
    std::vector<Plane> mPlanes;
    std::vector<Sphere> mSpheres;
};

}  // namespace Plato

#endif  // PLATO_THIRD_PARTY_INTEGRATION_KRINO_LEVELSET_PRIMITIVES
