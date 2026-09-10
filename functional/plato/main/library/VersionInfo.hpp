#ifndef PLATO_MAIN_LIBRARY_VERSIONINFO
#define PLATO_MAIN_LIBRARY_VERSIONINFO

#include <string>

namespace plato::main::library
{
/// @brief Return a string containing version and commit info using cmake variables.
[[nodiscard]] auto version_info() -> std::string;
}  // namespace plato::main::library

#endif
