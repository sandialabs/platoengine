#include "plato/services/ExternalLoggerFileSink.hpp"

#include <algorithm>
#include <boost/mpi/communicator.hpp>
#include <boost/shared_ptr.hpp>
#include <fstream>
#include <string>

#include "plato/components/ComponentTypeStream.hpp"
#include "plato/third_party_integration/boost_log/ComponentAttributes.hpp"
#include "plato/third_party_integration/boost_log/LogSource.hpp"
#include "plato/third_party_integration/boost_log/SinkWithAttributeFormattersAndFilters.hpp"
#include "plato/third_party_integration/boost_log/TimeStampAttribute.hpp"

namespace plato::services
{
[[nodiscard]] auto external_logger_file_sink(const std::filesystem::path& aLogFilePath)
    -> third_party_integration::boost_log::LoggerSinkSetupTeardown
{
    namespace tpi_bl = third_party_integration::boost_log;

    if (!aLogFilePath.parent_path().empty())
    {
        std::filesystem::create_directories(aLogFilePath.parent_path());
    }
    return tpi_bl::sink_with_attribute_formatters_and_filters<tpi_bl::LogSourceAttribute<tpi_bl::LogSource::kExternal>,
                                                              tpi_bl::TimeStampAttribute,
                                                              tpi_bl::ComponentTypeAndNameAttribute>(
        boost::make_shared<std::ofstream>(aLogFilePath, std::ios::app));
}

auto external_log_file_path(const components::ComponentType aComponentType,
                            const boost::mpi::communicator& aCommunicator) -> std::filesystem::path
{
    auto tComponentTypeAsString = components::to_string(aComponentType);
    std::replace(tComponentTypeAsString.begin(), tComponentTypeAsString.end(), ' ', '-');
    const auto tBasePath = std::filesystem::path{"logs"};
    return tBasePath /
           std::filesystem::path{tComponentTypeAsString + "-rank-" + std::to_string(aCommunicator.rank()) + ".txt"};
}
}  // namespace plato::services
