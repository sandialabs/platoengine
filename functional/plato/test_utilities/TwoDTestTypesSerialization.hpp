#ifndef PLATO_TEST_UTILITIES_TWODTYPESSERIALIZATION
#define PLATO_TEST_UTILITIES_TWODTYPESSERIALIZATION

#include <boost/serialization/array.hpp>

#include "plato/test_utilities/TwoDTestTypes.hpp"

namespace plato::test_utilities
{
template <typename Archive>
void serialize(Archive& aArchive, TwoDVector& aVector, const unsigned int /*version*/)
{
    aArchive& aVector.mData;
}
}  // namespace plato::test_utilities

#endif
