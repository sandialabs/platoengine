#ifndef PLATO_SERVICES_EXTERNALLOGGERFILESINK
#define PLATO_SERVICES_EXTERNALLOGGERFILESINK

#include <boost/mpi/communicator.hpp>
#include <filesystem>
#include <string_view>
#include <vector>

#include "plato/components/ComponentType.hpp"
#include "plato/third_party_integration/boost_log/LoggerSinkSetupTeardown.hpp"
#include "plato/utilities/EnumIndexing.hpp"

namespace plato::services
{
/// @brief Returns a name that is unique to the combination of @a aComponentType, @a aComponentName, and the MPI rank on
/// communicator @a aCommunicator.
[[nodiscard]] auto external_log_file_path(components::ComponentType aComponentType,
                                          const boost::mpi::communicator& aCommunicator) -> std::filesystem::path;

using ComponentExternalLoggerSinks = std::array<third_party_integration::boost_log::LoggerSinkSetupTeardown,
                                                utilities::number_of_enumerates<components::ComponentType>()>;

/// @brief Creates all component external sinks, which are meant to capture output from external libraries.
[[nodiscard]] auto component_external_logger_file_sinks() -> ComponentExternalLoggerSinks;

/// @brief Return the name of the directory containing the log files
[[nodiscard]] auto logging_directory_path() -> std::filesystem::path;

}  // namespace plato::services

#endif
