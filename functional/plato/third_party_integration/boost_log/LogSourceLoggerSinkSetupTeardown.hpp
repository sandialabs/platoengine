#ifndef PLATO_THIRD_PARTY_INTEGRATION_BOOST_LOG_LOGSOURCELOGGERSETUPTEARDOWN
#define PLATO_THIRD_PARTY_INTEGRATION_BOOST_LOG_LOGSOURCELOGGERSETUPTEARDOWN

#include <memory>

#include "plato/third_party_integration/boost_log/LogSource.hpp"
#include "plato/third_party_integration/boost_log/LoggerSinkSetupTeardown.hpp"

namespace plato::third_party_integration::boost_log
{
/// @brief An RAII-style class to manage a boost::log sink. On construction, this adds a new sink to the boost logging
/// core using the ostream given on construction. On destruction, the sink is removed from the logging core.
template <LogSource kLogSource>
class LogSourceLoggerSinkSetupTeardown
{
   public:
    LogSourceLoggerSinkSetupTeardown(const std::shared_ptr<std::ostream>& aStreamSink);

   private:
    LoggerSinkSetupTeardown mLoggerSetupTeardown;
};

using InternalLoggerSinkSetupTeardown = LogSourceLoggerSinkSetupTeardown<LogSource::kInternal>;
using ExternalLoggerSinkSetupTeardown = LogSourceLoggerSinkSetupTeardown<LogSource::kExternal>;

/// @brief Initializes the internal logger with std::cout.
/// @post An internal logger sink to std::cout will be active for the lifetime of the program.
void initialize_internal_console_sink();

}  // namespace plato::third_party_integration::boost_log

#endif
