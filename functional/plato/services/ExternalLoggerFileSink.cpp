#include "plato/services/ExternalLoggerFileSink.hpp"

#include <boost/shared_ptr.hpp>
#include <fstream>
#include <memory>

#include "plato/third_party_integration/boost_log/LogSource.hpp"
#include "plato/third_party_integration/boost_log/SinkWithAttributeFormattersAndFilters.hpp"

namespace plato::services
{
[[nodiscard]] auto external_logger_file_sink(const std::filesystem::path& aLogFilePath)
    -> third_party_integration::boost_log::LoggerSinkSetupTeardown
{
    namespace tpi_bl = third_party_integration::boost_log;

    return tpi_bl::sink_with_attribute_formatters_and_filters<tpi_bl::LogSourceAttribute<tpi_bl::LogSource::kExternal>>(
        boost::make_shared<std::ofstream>(aLogFilePath));
}
}  // namespace plato::services
