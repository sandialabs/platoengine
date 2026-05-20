#include "plato/third_party_integration/common/Vector3Hash.hpp"

#include <boost/container_hash/hash.hpp>
#include <boost/functional/hash.hpp>
#include <cstddef>

#include "plato/third_party_integration/common/Vector3.hpp"

namespace plato::third_party_integration::common
{
auto hash_value(const Coordinate& aCoordinate) -> std::size_t
{
    auto tHash = std::size_t{0};
    boost::hash_combine(tHash, aCoordinate.mX);
    boost::hash_combine(tHash, aCoordinate.mY);
    boost::hash_combine(tHash, aCoordinate.mZ);
    return tHash;
}
}  // namespace plato::third_party_integration::common
