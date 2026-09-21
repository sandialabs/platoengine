#ifndef PLATO_MAIN_LIBRARY_COMMANDLINEINPUTHANDLER
#define PLATO_MAIN_LIBRARY_COMMANDLINEINPUTHANDLER

#include <filesystem>
#include <string>
#include <vector>

#include "plato/main/library/ExitStatus.hpp"

namespace plato::main::library
{
///@brief Take the command line input arguments @a aArguments (not including the executable name "plato"), and determine
/// the course of action.
/// Currently:
///      "plato" tells the user to provide input and exits
///      "plato --help" displays all the known commands and exits
///      "plato input.i" will try to run plato on the given input
/// @return an enum representing an error code.
auto handle_input(const std::vector<std::string>& aArguments) -> ExitStatus;

namespace detail
{

///@brief Run plato on the given input deck @a aInputFile
///@pre @a aInputFile exists on disk, Kokkos has been initialized
/// @return an enum representing an error code.
auto run_plato(const std::filesystem::path& aInputFile) -> ExitStatus;

///@brief Helper function to print an error message telling the user to provide an input file or request --help
void print_command_line_error_message();

///@brief Helper function to print all the known inputs that are defined in the input block structs.
void print_known_inputs();

///@brief Helper function to print the version and commit of platoengine
void print_version();

}  // namespace detail

}  // namespace plato::main::library

#endif
