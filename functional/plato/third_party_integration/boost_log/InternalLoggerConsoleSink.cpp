#include "plato/third_party_integration/boost_log/InternalLoggerConsoleSink.hpp"

#include <boost/core/null_deleter.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/expressions/formatters/stream.hpp>
#include <iostream>
#include <memory>

#include "plato/third_party_integration/boost_log/AttributeFormatter.hpp"
#include "plato/third_party_integration/boost_log/ComponentAttributes.hpp"
#include "plato/third_party_integration/boost_log/FilterConjunction.hpp"
#include "plato/third_party_integration/boost_log/LogSource.hpp"
#include "plato/third_party_integration/boost_log/MPIAttributes.hpp"
#include "plato/third_party_integration/boost_log/Severity.hpp"
#include "plato/third_party_integration/boost_log/TimeStampAttribute.hpp"

namespace plato::third_party_integration::boost_log
{
auto internal_console_sink(const std::shared_ptr<std::ostream>& aStreamSink) -> LoggerSinkSetupTeardown
{
    auto tMessageFormatter =
        boost::log::formatter{boost::log::expressions::stream << boost::log::expressions::smessage};
    return LoggerSinkSetupTeardown{
        aStreamSink,
        attribute_formatter(time_stamp_formatter(), ComponentTypeAndNameAttribute::formatter(),
                            severity_attribute_formatter(), std::move(tMessageFormatter)),
        filter_conjunction(LogSourceAttribute<LogSource::kInternal>::filter(), MPIWorldCommRankAttribute::filter())};
}

auto internal_console_sink() -> LoggerSinkSetupTeardown
{
    return internal_console_sink(std::shared_ptr<std::ostream>{&std::cout, boost::null_deleter()});
}
}  // namespace plato::third_party_integration::boost_log
