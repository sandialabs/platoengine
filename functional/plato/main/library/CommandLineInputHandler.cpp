#include "plato/main/library/CommandLineInputHandler.hpp"

#include <boost/mpi/communicator.hpp>
#include <filesystem>
#include <iostream>
#include <string_view>

#include "plato/input_parser/HelpDocumentationRegistration.hpp"
#include "plato/main/library/Executor.hpp"
#include "plato/process_manager/library/ProcessManagerData.hpp"
#include "plato/process_manager/library/ProcessManagerFactory.hpp"
#include "plato/process_manager/library/ProcessManagerRegistration.hpp"
#include "plato/process_manager/library/ValidatedInput.hpp"
#include "plato/utilities/Exception.hpp"
#include "plato/utilities/StringUtilities.hpp"

namespace plato::main::library
{
namespace
{
constexpr auto kHelpKey = std::string_view{"--help"};

void unregistered_process_manager_warning(
    const process_manager::library::ValidatedProcessManagerInputVector& aValidatedProcessManagerInput)
{
    const auto tUnregisteredProcessManagers =
        process_manager::library::unregistered_process_managers(aValidatedProcessManagerInput);
    if (!tUnregisteredProcessManagers.empty())
    {
        std::cout << "Warning: The following process managers are in the input, but are not implemented in this build "
                     "of plato:\n";
        std::cout << utilities::concatenate_container(tUnregisteredProcessManagers, "\n");
        std::cout << "\nThese process managers will be skipped.\n";
    }
}
}  // namespace

void handle_input(const std::vector<std::string>& aArguments)
{
    if (!aArguments.empty() && aArguments.front() == std::string{kHelpKey})
    {
        detail::print_known_inputs();
    }
    else if (aArguments.empty() || !std::filesystem::exists(aArguments.front()))
    {
        detail::print_error_message();
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
    try
    {
        namespace ppml = plato::process_manager::library;
        const auto tValidatedInput = ppml::parse_and_validate_from_file(aInputFile);
        unregistered_process_manager_warning(tValidatedInput.processManagers());
        const auto tExecutor =
            plato::main::library::Executor{ppml::make_process_managers(tValidatedInput.processManagers())};
        const auto tProcessManagerData = ppml::make_process_manager_data(tValidatedInput);
        tExecutor.execute(tProcessManagerData);
    }
    catch (const plato::utilities::Exception& tError)
    {
        print_message(tError.what());
    }
}

void print_message(const std::string_view aMessage)
{
    if (boost::mpi::communicator{}.rank() == 0)
    {
        std::cout << aMessage << std::endl;
    }
}

void print_error_message()
{
    print_message(
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
