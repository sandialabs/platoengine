#include "plato/services/InternalLoggerConsoleSink.hpp"

#include <boost/core/null_deleter.hpp>
#include <iostream>
#include <memory>

#include "plato/third_party_integration/boost_log/ComponentAttributes.hpp"
#include "plato/third_party_integration/boost_log/LogSource.hpp"
#include "plato/third_party_integration/boost_log/MPIAttributes.hpp"
#include "plato/third_party_integration/boost_log/Severity.hpp"
#include "plato/third_party_integration/boost_log/SinkWithAttributeFormattersAndFilters.hpp"
#include "plato/third_party_integration/boost_log/TimeStampAttribute.hpp"

namespace plato::services
{
auto internal_logger_console_sink(const boost::shared_ptr<std::ostream>& aStreamSink)
    -> third_party_integration::boost_log::LoggerSinkSetupTeardown
{
    namespace tpi_bl = third_party_integration::boost_log;

    return tpi_bl::sink_with_attribute_formatters_and_filters<
        tpi_bl::TimeStampAttribute, tpi_bl::ComponentTypeAndNameAttribute, tpi_bl::SeverityAttribute,
        tpi_bl::LogSourceAttribute<tpi_bl::LogSource::kInternal>, tpi_bl::MPIWorldCommRankAttribute>(aStreamSink);
}

auto internal_logger_console_sink() -> third_party_integration::boost_log::LoggerSinkSetupTeardown
{
    return internal_logger_console_sink(boost::shared_ptr<std::ostream>{&std::cout, boost::null_deleter()});
}
}  // namespace plato::services
