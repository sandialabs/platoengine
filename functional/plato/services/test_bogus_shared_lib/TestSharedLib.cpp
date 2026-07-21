#include "plato/services/test_shared_lib/TestSharedLib.hpp"

#include <iostream>

namespace plato::services::test_shared_lib
{
auto my_round(const double /*aValue*/) -> long int { return 0L; }

void unload_library() { std::cout << "Bogus library unloading.\n"; }
}  // namespace plato::services::test_shared_lib
