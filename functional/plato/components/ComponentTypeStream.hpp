#ifndef PLATO_COMPONENTS_COMPONENTTYPESTREAM
#define PLATO_COMPONENTS_COMPONENTTYPESTREAM

#include <ostream>

#include "plato/components/ComponentType.hpp"

namespace plato::components
{
/// @brief Stream insertion for a ComponentType.
auto operator<<(std::ostream& aStream, ComponentType aComponentType) -> std::ostream&;
}  // namespace plato::components

#endif
