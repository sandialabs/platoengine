#include "plato/test_utilities/TestDataFilePath.hpp"

namespace plato::test_utilities
{

std::optional<std::filesystem::path> test_data_file_path(const std::string_view aFileName)
{
    static const auto tBuildPath = std::filesystem::path{"@FUNCTIONAL_TEST_DATA_BUILD_PATH@"};
    static const auto tInstallPath = std::filesystem::path{"@FUNCTIONAL_TEST_DATA_INSTALL_PATH@"};

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

}  // namespace plato::test_utilities
