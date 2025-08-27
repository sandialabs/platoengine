#ifndef PLATO_SERVICES_EXTERNALLOGGERFILESINKDETAIL
#define PLATO_SERVICES_EXTERNALLOGGERFILESINKDETAIL

#include <filesystem>

#include "plato/third_party_integration/boost_log/ComponentAttributes.hpp"
#include "plato/third_party_integration/boost_log/LogSource.hpp"
#include "plato/third_party_integration/boost_log/LoggerSinkSetupTeardown.hpp"
#include "plato/third_party_integration/boost_log/SinkWithAttributeFormattersAndFilters.hpp"
#include "plato/third_party_integration/boost_log/TimeStampAttribute.hpp"

namespace plato::services::detail
{
template <components::ComponentType kComponentType>
[[nodiscard]] auto external_logger_file_sink(const std::filesystem::path& aLogFilePath)
    -> third_party_integration::boost_log::LoggerSinkSetupTeardown
{
    namespace tpi_bl = third_party_integration::boost_log;

    if (!aLogFilePath.parent_path().empty())
    {
        std::filesystem::create_directories(aLogFilePath.parent_path());
    }
    return tpi_bl::sink_with_attribute_formatters_and_filters<
        tpi_bl::LogSourceAttribute<tpi_bl::LogSource::kExternal>, tpi_bl::TimeStampAttribute,
        tpi_bl::ComponentTypeAndNameAttribute, tpi_bl::ComponentTypeFilterAttribute<kComponentType>>(
        boost::make_shared<std::ofstream>(aLogFilePath, std::ios::app));
}
}  // namespace plato::services::detail

#endif
