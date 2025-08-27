#ifndef PLATO_MAIN_COMMANDLINEINPUTHANDLER
#define PLATO_MAIN_COMMANDLINEINPUTHANDLER

#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

namespace plato::main::library
{

///@brief Take the command line input arguments @a aArguments (not including the executable name "plato"), and determine
/// the course of action.
/// Currently:
///      "plato" tells the user to provide input and exits
///      "plato --help" displays all the known commands and exits
///      "plato input.i" will try to run plato on the given input
void handle_input(const std::vector<std::string>& aArguments);

namespace detail
{

///@brief Run plato on the given input deck @a aInputFile
///@pre @a aInputFile exists on disk, Kokkos has been initialized
void run_plato(const std::filesystem::path& aInputFile);

///@brief Helper function to print an error message telling the user to provide an input file or request --help
void print_command_line_error_message();

///@brief Helper function to print all the known inputs that are defined in the input block structs.
void print_known_inputs();

}  // namespace detail

}  // namespace plato::main::library

#endif
