#include "plato/third_party_integration/boost_log/InternalLoggerConsoleSink.hpp"

#include <boost/core/null_deleter.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/expressions/formatters/stream.hpp>
#include <iostream>
#include <memory>

#include "plato/third_party_integration/boost_log/FilterConjunction.hpp"
#include "plato/third_party_integration/boost_log/LogSource.hpp"
#include "plato/third_party_integration/boost_log/MPIAttributes.hpp"

namespace plato::third_party_integration::boost_log
{
auto internal_console_sink() -> LoggerSinkSetupTeardown
{
    return LoggerSinkSetupTeardown{
        std::shared_ptr<std::ostream>{&std::cout, boost::null_deleter()},
        boost::log::formatter{boost::log::expressions::stream << boost::log::expressions::smessage},
        filter_conjunction(log_source_filter(LogSource::kInternal), mpi_root_rank_filter())};
};
}  // namespace plato::third_party_integration::boost_log
