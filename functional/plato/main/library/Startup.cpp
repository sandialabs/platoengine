#include "plato/main/library/Startup.hpp"

#include "plato/criteria/extension/Startup.hpp"
#include "plato/services/InternalLoggerConsoleSink.hpp"

namespace plato::main::library
{
void startup()
{
    [[maybe_unused]] const auto tInternalLogSink = services::internal_logger_console_sink();
    criteria::extension::startup();
}

}  // namespace plato::main::library
