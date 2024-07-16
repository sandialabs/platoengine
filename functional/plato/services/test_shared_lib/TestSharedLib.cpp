#include "plato/services/test_shared_lib/TestSharedLib.hpp"

#include <cmath>

namespace plato::services::test_shared_lib
{
long int my_round(const double aValue) { return std::lround(aValue); }
}  // namespace plato::services::test_shared_lib
