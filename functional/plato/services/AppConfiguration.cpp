#include "plato/services/AppConfiguration.hpp"

#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/core/nvp.hpp>
#include <fstream>

namespace plato::services
{
namespace
{
constexpr auto kXMLName = std::string_view{"plato_app_configuration"};
}

void save_configuration(const AppConfiguration& aAppConfiguration, const std::filesystem::path& aFilename)
{
    auto tOutFileStream = std::ofstream{aFilename};
    auto tOutputArchive = boost::archive::xml_oarchive{tOutFileStream};
    tOutputArchive << boost::serialization::make_nvp(kXMLName.data(), aAppConfiguration);
}

AppConfiguration load_configuration(const std::filesystem::path& aFilename)
{
    auto tInFileStream = std::ifstream{aFilename};
    auto tInputArchive = boost::archive::xml_iarchive{tInFileStream};
    auto tAppConfiguration = AppConfiguration{};
    tInputArchive >> boost::serialization::make_nvp(kXMLName.data(), tAppConfiguration);
    return tAppConfiguration;
}

bool operator==(const CriterionConfiguration& aCriterionConfigurationLeft,
                const CriterionConfiguration& aCriterionConfigurationRight)
{
    return aCriterionConfigurationLeft.mName == aCriterionConfigurationRight.mName &&
           aCriterionConfigurationLeft.mFunctionName == aCriterionConfigurationRight.mFunctionName &&
           aCriterionConfigurationLeft.mIsParallelized == aCriterionConfigurationRight.mIsParallelized;
}

bool operator==(const AppConfiguration& aAppConfigurationLeft, const AppConfiguration& aAppConfigurationRight)
{
    return aAppConfigurationLeft.mName == aAppConfigurationRight.mName &&
           aAppConfigurationLeft.mLibraryFileName == aAppConfigurationRight.mLibraryFileName &&
           aAppConfigurationLeft.mCriteria == aAppConfigurationRight.mCriteria;
}
}  // namespace plato::services
