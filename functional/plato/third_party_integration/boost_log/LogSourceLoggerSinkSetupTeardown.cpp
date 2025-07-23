#include "plato/third_party_integration/boost_log/LogSourceLoggerSinkSetupTeardown.hpp"

#include <boost/core/null_deleter.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/expressions/filter.hpp>
#include <iostream>

#include "plato/third_party_integration/boost_log/LogSource.hpp"

namespace plato::third_party_integration::boost_log
{
template <LogSource kLogSource>
LogSourceLoggerSinkSetupTeardown<kLogSource>::LogSourceLoggerSinkSetupTeardown(
    const std::shared_ptr<std::ostream>& aStreamSink)
    : mLoggerSetupTeardown{
          aStreamSink, boost::log::formatter{boost::log::expressions::stream << boost::log::expressions::smessage},
          boost::log::filter{boost::log::expressions::has_attr(log_source) && log_source == kLogSource}}
{
}

void initialize_internal_console_sink()
{
    [[maybe_unused]] static auto tConsoleSink = LogSourceLoggerSinkSetupTeardown<LogSource::kInternal>{
        std::shared_ptr<std::ostream>{&std::cout, boost::null_deleter()}};
}

template class LogSourceLoggerSinkSetupTeardown<LogSource::kInternal>;

template class LogSourceLoggerSinkSetupTeardown<LogSource::kExternal>;

}  // namespace plato::third_party_integration::boost_log
