#ifndef PLATO_SERVICES_EXTERNALLOGGERFILESINK
#define PLATO_SERVICES_EXTERNALLOGGERFILESINK

#include <boost/mpi/communicator.hpp>
#include <filesystem>
#include <string_view>

#include "plato/components/ComponentType.hpp"
#include "plato/third_party_integration/boost_log/LoggerSinkSetupTeardown.hpp"

namespace plato::services
{
/// @brief Creates a log sink to the file at path @a aLogFile. The sink always opens the file in append mode, or creates
/// a new file if it doesn't exist.
[[nodiscard]] auto external_logger_file_sink(const std::filesystem::path& aLogFilePath)
    -> third_party_integration::boost_log::LoggerSinkSetupTeardown;

/// @brief Returns a name that is unique to the combination of @a aComponentType, @a aComponentName, and the MPI rank on
/// communicator @a aCommunicator.
[[nodiscard]] auto external_log_file_path(components::ComponentType aComponentType,
                                          const boost::mpi::communicator& aCommunicator) -> std::filesystem::path;
}  // namespace plato::services

#endif
