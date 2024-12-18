#include "plato/utilities/ArgcArgvToVector.hpp"

namespace plato::utilities
{

std::vector<std::string> argc_argv_to_std_vector(int aArgc, char** aArgv)
{
    std::vector<std::string> tArguments;
    for (int tIndex = 1; tIndex < aArgc; ++tIndex)
    {
        tArguments.emplace_back(aArgv[tIndex]);
    }
    return tArguments;
}

}  // namespace plato::utilities
