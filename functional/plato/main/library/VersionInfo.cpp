#include "plato/main/library/VersionInfo.hpp"

#include <format>
#include <string>

#include "plato/main/library/BuildInfo.hpp"

namespace plato::main::library
{
auto version_info() -> std::string
{
    return std::format("Version {} : commit \"{}\"", project_version(), git_commit_hash());
}
}  // namespace plato::main::library
