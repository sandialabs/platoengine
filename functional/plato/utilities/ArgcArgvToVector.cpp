#include "plato/utilities/ArgcArgvToVector.hpp"

namespace plato::utilities
{

auto argc_argv_to_std_vector(const int aArgc, const char* const* const aArgv) -> std::vector<std::string>
{
    const auto tProgramArguments = std::span(aArgv, static_cast<std::size_t>(aArgc)) | std::views::drop(1U);
    return std::vector<std::string>(tProgramArguments.begin(), tProgramArguments.end());
}

}  // namespace plato::utilities
