#ifndef PLATO_THIRDPARTYINTEGRATION_COMMON_VECTOR3HASH
#define PLATO_THIRDPARTYINTEGRATION_COMMON_VECTOR3HASH

#include <boost/functional/hash.hpp>

#include "plato/third_party_integration/common/Vector3.hpp"

namespace plato::third_party_integration::common
{
std::size_t hash_value(const Coordinate& aCoordinate)
{
    auto tHash = std::size_t{0};
    boost::hash_combine(tHash, aCoordinate.x);
    boost::hash_combine(tHash, aCoordinate.y);
    boost::hash_combine(tHash, aCoordinate.z);
    return tHash;
}
}  // namespace plato::third_party_integration::common

#endif
