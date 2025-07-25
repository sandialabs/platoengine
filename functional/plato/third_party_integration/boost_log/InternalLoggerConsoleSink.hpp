#ifndef PLATO_THIRD_PARTY_INTEGRATION_BOOST_LOG_INTERNALLOGGERCONSOLESINK
#define PLATO_THIRD_PARTY_INTEGRATION_BOOST_LOG_INTERNALLOGGERCONSOLESINK

#include "plato/third_party_integration/boost_log/LoggerSinkSetupTeardown.hpp"

namespace plato::third_party_integration::boost_log
{
/// @brief Returns a sink that streams to the console via `std::cout`.
/// @note This sink is filtered to only output messages tagged with `LoggerSource::kInternal` and mpi rank 0.
[[nodiscard]] auto internal_console_sink() -> LoggerSinkSetupTeardown;

}  // namespace plato::third_party_integration::boost_log

#endif
