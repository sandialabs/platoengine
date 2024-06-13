#include "plato/utilities/FileUtilities.hpp"

#include <boost/filesystem.hpp>
#include <boost/filesystem/operations.hpp>

namespace plato::utilities
{
std::filesystem::path make_filename_unique(std::filesystem::path aBasePath)
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
