#ifndef PLATO_CRITERIA_EXTENSION_STARTUP
#define PLATO_CRITERIA_EXTENSION_STARTUP

#include <cstddef>

namespace plato::criteria::extension
{
/// @brief Must be called at the start of the program, this mainly loads plugins.
auto startup() -> std::size_t;

}  // namespace plato::criteria::extension

#endif
