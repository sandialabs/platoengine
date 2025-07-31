#ifndef PLATO_COMPONENTS_COMPONENTTYPECOLORS
#define PLATO_COMPONENTS_COMPONENTTYPECOLORS

#include "plato/components/ComponentType.hpp"
#include "plato/utilities/Colorize.hpp"

namespace plato::components
{
/// @brief Returns a TextColor for colorizing the console output of components in a standard way.
[[nodiscard]] auto component_text_color(ComponentType aComponentType) -> utilities::TextColor;
}  // namespace plato::components

#endif
