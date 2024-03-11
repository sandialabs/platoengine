#ifndef PLATO_LINEAR_ALGEBRA_DYNAMICVECTORSERIALIZATION
#define PLATO_LINEAR_ALGEBRA_DYNAMICVECTORSERIALIZATION

#include <boost/serialization/vector.hpp>

#include "plato/linear_algebra/DynamicVector.hpp"

namespace plato::linear_algebra
{
template <typename Archive, typename U>
void serialize(Archive& aArchive, DynamicVector<U>& aVector, const unsigned int /*aVersion*/)
{
    aArchive& aVector.mVector;
}

}  // namespace plato::linear_algebra

#endif
