#ifndef PLATO_THIRD_PARTY_INTEGRATION_BOOST_LOG_INTERNALLOGGERCONSOLESINK
#define PLATO_THIRD_PARTY_INTEGRATION_BOOST_LOG_INTERNALLOGGERCONSOLESINK

#include <ostream>

#include "plato/third_party_integration/boost_log/LoggerSinkSetupTeardown.hpp"

namespace plato::third_party_integration::boost_log
{
/// @brief Returns a sink that streams to @a aStream and sets up filters and formatters.
///
/// This sink is filtered to only output messages tagged with `LoggerSource::kInternal` and mpi rank 0.
/// The formatter can handle attributes: Severity, ComponentTypeAndName, and stamps.
[[nodiscard]] auto internal_console_sink(const std::shared_ptr<std::ostream>& aStreamSink) -> LoggerSinkSetupTeardown;

/// @brief This overload returns a sink that streams to `std::cout`.
///
/// This sink is filtered to only output messages tagged with `LoggerSource::kInternal` and mpi rank 0.
/// The formatter can handle attributes: Severity, ComponentTypeAndName, and stamps.
[[nodiscard]] auto internal_console_sink() -> LoggerSinkSetupTeardown;

}  // namespace plato::third_party_integration::boost_log

#endif
