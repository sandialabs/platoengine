#include "plato/utilities/FileUtilities.hpp"

#include <boost/filesystem/operations.hpp>
#include <filesystem>

namespace plato::utilities
{
auto make_filename_unique(std::filesystem::path aBasePath) -> std::filesystem::path
{
    if (!aBasePath.has_filename())
    {
        return aBasePath;
    }
    const auto tUniqueName = boost::filesystem::unique_path();
    const auto tNewFileName = aBasePath.stem().string() + "-" + tUniqueName.string() + aBasePath.extension().string();
    aBasePath.replace_filename(tNewFileName);
    return aBasePath;
}

}  // namespace plato::utilities
