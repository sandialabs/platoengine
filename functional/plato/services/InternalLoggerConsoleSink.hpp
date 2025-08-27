#ifndef PLATO_SERVICES_INTERNALLOGGERCONSOLESINK
#define PLATO_SERVICES_INTERNALLOGGERCONSOLESINK

#include <boost/shared_ptr.hpp>
#include <ostream>

#include "plato/third_party_integration/boost_log/LoggerSinkSetupTeardown.hpp"

namespace plato::services
{
/// @brief Returns a sink that streams to @a aStream and sets up filters and formatters.
///
/// This sink is filtered to only output messages tagged with `LoggerSource::kInternal` and mpi rank 0.
/// The formatter can handle attributes: Severity, ComponentTypeAndName, and time stamps.
[[nodiscard]] auto internal_logger_console_sink(const boost::shared_ptr<std::ostream>& aStreamSink)
    -> third_party_integration::boost_log::LoggerSinkSetupTeardown;

/// @brief This overload returns a sink that streams to `std::cout`.
///
/// This sink is filtered to only output messages tagged with `LoggerSource::kInternal` and mpi rank 0.
/// The formatter can handle attributes: Severity, ComponentTypeAndName, and time stamps.
[[nodiscard]] auto internal_logger_console_sink() -> third_party_integration::boost_log::LoggerSinkSetupTeardown;

}  // namespace plato::services

#endif
