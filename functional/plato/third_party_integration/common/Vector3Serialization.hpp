#ifndef PLATO_THIRDPARTYINTEGRATION_COMMON_VECTOR3SERIALIZATION
#define PLATO_THIRDPARTYINTEGRATION_COMMON_VECTOR3SERIALIZATION

#include <boost/serialization/vector.hpp>

#include "plato/third_party_integration/common/Vector3.hpp"

namespace plato::third_party_integration::common
{
template <typename Archive>
void serialize(Archive& aArchive, Vector3& aVector, const unsigned int /*aVersion*/)
{
    aArchive & aVector.mX;
    aArchive & aVector.mY;
    aArchive & aVector.mZ;
}
}  // namespace plato::third_party_integration::common
#endif
