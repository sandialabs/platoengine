#ifndef PLATO_SERVICES_EXTERNALLOGGERFILESINK
#define PLATO_SERVICES_EXTERNALLOGGERFILESINK

#include <filesystem>

#include "plato/third_party_integration/boost_log/LoggerSinkSetupTeardown.hpp"

namespace plato::services
{
/// @brief Creates a log sink to the file at path @a aLogFile. The sink always opens the file in append mode, or creates
/// a new file if it doesn't exist.
[[nodiscard]] auto external_logger_file_sink(const std::filesystem::path& aLogFilePath)
    -> third_party_integration::boost_log::LoggerSinkSetupTeardown;
}  // namespace plato::services

#endif
