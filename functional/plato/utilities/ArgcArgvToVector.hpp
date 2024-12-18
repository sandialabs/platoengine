#ifndef PLATO_UTILITIES_ARGCARGVTOVECTOR
#define PLATO_UTILITIES_ARGCARGVTOVECTOR

#include <string>
#include <vector>

namespace plato::utilities
{

///@brief Utility to convert c-style main input of @a aArgc, and @a aArgv, into a vector of strings
[[nodiscard]] std::vector<std::string> argc_argv_to_std_vector(int aArgc, char** aArgv);

}  // namespace plato::utilities

#endif
