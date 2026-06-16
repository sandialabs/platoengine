#ifndef PLATO_THIRDPARTYINTEGRATION_COMMON_VECTOR3HASH
#define PLATO_THIRDPARTYINTEGRATION_COMMON_VECTOR3HASH

#include "plato/third_party_integration/common/Vector3.hpp"

namespace plato::third_party_integration::common
{
/// @brief Overload used by boost::hash_combine
[[nodiscard]] auto hash_value(const Coordinate& aCoordinate) -> std::size_t;
}  // namespace plato::third_party_integration::common

#endif
