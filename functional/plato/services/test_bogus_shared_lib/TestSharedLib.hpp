#ifndef PLATO_SERVICES_TESTSHAREDLIB_TESTSHAREDLIB
#define PLATO_SERVICES_TESTSHAREDLIB_TESTSHAREDLIB

#include <memory>

namespace plato::services::test_shared_lib
{
/// @brief Returns 0 regardless of input.
/// @note This is only for testing shared library loading.
extern "C" long int my_round(double aValue);

}  // namespace plato::services::test_shared_lib

#endif
