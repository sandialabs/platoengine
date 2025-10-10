#include "plato/main/library/CommandLineInputHandler.hpp"

#include <boost/mpi/communicator.hpp>
#include <filesystem>
#include <iostream>
#include <string_view>

#include "plato/input_parser/HelpDocumentationRegistration.hpp"
#include "plato/input_validation/ValidatedInput.hpp"
#include "plato/main/library/Executor.hpp"
#include "plato/main/library/Splash.hpp"
#include "plato/process_manager/library/ProcessManagerData.hpp"
#include "plato/process_manager/library/ProcessManagerFactory.hpp"
#include "plato/process_manager/library/ProcessManagerRegistration.hpp"
#include "plato/services/ExternalLoggerFileSink.hpp"
#include "plato/services/InternalLoggerConsoleSink.hpp"
#include "plato/services/SystemLogger.hpp"
#include "plato/utilities/Exception.hpp"
#include "plato/utilities/MPIUtilities.hpp"
#include "plato/utilities/StringUtilities.hpp"

namespace plato::main::library
{
namespace
{
constexpr auto kHelpKey = std::string_view{"--help"};
}  // namespace

void handle_input(const std::vector<std::string>& aArguments)
{
    plato::utilities::execute_on_root(boost::mpi::communicator{},
                                      []() { std::filesystem::remove_all(plato::services::logging_directory_path()); });
    [[maybe_unused]] const auto tInternalLogSink = services::internal_logger_console_sink();
    [[maybe_unused]] const auto tExternalLogSinks = services::component_external_logger_file_sinks();

    if (!aArguments.empty() && aArguments.front() == std::string{kHelpKey})
    {
        detail::print_known_inputs();
    }
    else if (aArguments.empty() || !std::filesystem::exists(aArguments.front()))
    {
        detail::print_command_line_error_message();
    }
    else
    {
        detail::run_plato(aArguments.front());
    }
}

namespace detail
{

void run_plato(const std::filesystem::path& aInputFile)
{
    const auto tValidatedInput = input_validation::parse_and_validate_file(aInputFile);
    if (tValidatedInput.hasError())
    {
        services::system_logger().logError(tValidatedInput.error());
        return;
    }

    if (boost::mpi::communicator{}.rank() == 0)
    {
        print_splash_screen(std::cout);
    }

    const auto& tValidatedProcessManagers = tValidatedInput.value().get<components::ComponentType::kProcessManager>();
    const auto tExecutor =
        plato::main::library::Executor{process_manager::library::make_process_managers(tValidatedProcessManagers)};
    const auto tProcessManagerData = process_manager::library::make_process_manager_data(tValidatedInput.value());

    try
    {
        tExecutor.execute(tProcessManagerData);
    }
    catch (const std::exception& tError)
    {
        services::system_logger().logError("Plato encountered an exception, exiting.");
        services::system_logger().logError(tError.what());
    }
}

void print_command_line_error_message()
{
    services::system_logger().logError(
        "Executable expects an input file name as an argument. Type 'plato --help' for additional information. "
        "Aborting.");
}

void print_known_inputs()
{
    if (boost::mpi::communicator{}.rank() == 0)
    {
        plato::input_parser::known_inputs(std::cout);
    }
}

}  // namespace detail
}  // namespace plato::main::library
