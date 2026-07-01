#include "plato/utilities/DataFilePath.hpp"

#include <filesystem>

namespace plato::utilities
{

auto data_file_path(const std::filesystem::path& aFileName) -> std::optional<std::filesystem::path>
{
    static const auto tBuildPath = std::filesystem::path{"@FUNCTIONAL_DATA_BUILD_PATH@"};
    static const auto tInstallPath = std::filesystem::path{"@FUNCTIONAL_DATA_INSTALL_PATH@"};

    if (std::filesystem::exists(tBuildPath / aFileName))
    {
        return tBuildPath / aFileName;
    }
    if (std::filesystem::exists(tInstallPath / aFileName))
    {
        return tInstallPath / aFileName;
    }
    return std::nullopt;
}

}  // namespace plato::utilities
