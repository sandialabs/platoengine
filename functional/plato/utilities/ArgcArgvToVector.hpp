#ifndef PLATO_UTILITIES_ARGCARGVTOVECTOR
#define PLATO_UTILITIES_ARGCARGVTOVECTOR

#include <string>
#include <vector>

namespace plato::utilities
{

///@brief Utility to convert c-style main input of @a aArgc, and @a aArgv, into a vector of strings
[[nodiscard]] auto argc_argv_to_std_vector(int aArgc, const char* const* aArgv) -> std::vector<std::string>;

}  // namespace plato::utilities

#endif
