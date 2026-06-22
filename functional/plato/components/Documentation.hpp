#ifndef PLATO_COMPONENTS_DOCUMENTATION
#define PLATO_COMPONENTS_DOCUMENTATION

#include <string>

#include "plato/components/ComponentType.hpp"

namespace plato::components
{
/// Returns a brief documentation blurb for @a aComponentType.
[[nodiscard]] auto component_documentation_brief(ComponentType aComponentType) -> std::string;
}  // namespace plato::components

#endif
