#ifndef PLATO_SERVICES_APPCONFIGURATION
#define PLATO_SERVICES_APPCONFIGURATION

#include <boost/core/nvp.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/vector.hpp>
#include <filesystem>
#include <string>
#include <vector>

namespace plato::services
{
/// @brief Configuration data for defining a single criterion's interface.
///
/// This includes the function name that is loaded from the shared library as well
/// as if the function has serial or parallel implementations and is a vector or scalar function.
struct CriterionConfiguration
{
    std::string mName{};
    bool mIsParallelized = false;
    bool mIsScalar = true;
    std::string mFunctionName{};
    auto operator==(const CriterionConfiguration& aCriterionConfiguration) const -> bool = default;
};

/// @brief Configuration data for defining an app's interface with plato.
///
/// A file, written using the serialize function is expected to be associated with each
/// shared library containing an app.
struct AppConfiguration
{
    std::string mName{};
    std::string mLibraryFileName{};
    std::vector<CriterionConfiguration> mCriteria{};
    auto operator==(const AppConfiguration& aAppConfiguration) const -> bool = default;
};

/// @brief Associates an AppConfiguration with the directory in which it was found.
///
/// The purpose of this is to generate a path for the shared library associated with the
/// AppConfiguration, `mLibraryFileName`.
struct AppConfigurationWithDirectory
{
    AppConfiguration mConfiguration;
    std::filesystem::path mLibraryDirectory;
};

/// @brief Writes @a aAppConfiguration to disk, at path @a aFilename.
void save_configuration(const AppConfiguration& aAppConfiguration, const std::filesystem::path& aFilename);

/// @brief Reads an AppConfiguration from disk, at path @a aFilename.
[[nodiscard]] AppConfiguration load_configuration(const std::filesystem::path& aFilename);

template <class Archive>
void serialize(Archive& aArchive, CriterionConfiguration& aAppConfiguration, const unsigned int /*version*/)
{
    aArchive& boost::serialization::make_nvp("name", aAppConfiguration.mName);
    aArchive& boost::serialization::make_nvp("exported_function", aAppConfiguration.mFunctionName);
    aArchive& boost::serialization::make_nvp("is_parallelized", aAppConfiguration.mIsParallelized);
    aArchive& boost::serialization::make_nvp("is_scalar", aAppConfiguration.mIsScalar);
}

template <class Archive>
void serialize(Archive& aArchive, AppConfiguration& aAppConfiguration, const unsigned int /*version*/)
{
    aArchive& boost::serialization::make_nvp("name", aAppConfiguration.mName);
    aArchive& boost::serialization::make_nvp("shared_library_file_name", aAppConfiguration.mLibraryFileName);
    aArchive& boost::serialization::make_nvp("criteria", aAppConfiguration.mCriteria);
}

}  // namespace plato::services

#endif
