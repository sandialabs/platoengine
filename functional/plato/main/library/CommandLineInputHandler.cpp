#include "plato/main/library/CommandLineInputHandler.hpp"

#include <boost/mpi/communicator.hpp>
#include <filesystem>
#include <iostream>
#include <string_view>

#include "plato/input_parser/HelpDocumentationRegistration.hpp"
#include "plato/input_validation/ValidatedInput.hpp"
#include "plato/main/library/Executor.hpp"
#include "plato/process_manager/library/ProcessManagerData.hpp"
#include "plato/process_manager/library/ProcessManagerFactory.hpp"
#include "plato/process_manager/library/ProcessManagerRegistration.hpp"
#include "plato/utilities/Exception.hpp"
#include "plato/utilities/StringUtilities.hpp"

namespace plato::main::library
{
namespace
{
constexpr auto kHelpKey = std::string_view{"--help"};
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
    const auto tValidatedInput = input_validation::parse_and_validate_file(aInputFile);
    if (tValidatedInput.hasError())
    {
        print_message(tValidatedInput.error());
        return;
    }
    const auto& tValidatedProcessManagers = tValidatedInput.value().get<input_parser::ComponentType::kProcessManager>();
    const auto tExecutor =
        plato::main::library::Executor{process_manager::library::make_process_managers(tValidatedProcessManagers)};
    const auto tProcessManagerData = process_manager::library::make_process_manager_data(tValidatedInput.value());

    try
    {
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
