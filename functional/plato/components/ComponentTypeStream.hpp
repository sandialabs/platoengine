#ifndef PLATO_COMPONENTS_COMPONENTTYPESTREAM
#define PLATO_COMPONENTS_COMPONENTTYPESTREAM

#include <ostream>

#include "plato/components/ComponentType.hpp"

namespace plato::components
{
/// @brief Stream insertion for a ComponentType.
auto operator<<(std::ostream& aStream, ComponentType aComponentType) -> std::ostream&;

/// @brief Converts @a aComponentType to a string.
[[nodiscard]] auto to_string(ComponentType aComponentType) -> std::string;

}  // namespace plato::components

#endif
