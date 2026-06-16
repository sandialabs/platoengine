#ifndef PLATO_THIRD_PARTY_INTEGRATION_SNOPT_SNOPTARRAY
#define PLATO_THIRD_PARTY_INTEGRATION_SNOPT_SNOPTARRAY

#include <span>

namespace plato::third_party_integration::snopt
{
template <typename T>
using SNOPTArray = std::span<T>;
}

#endif
