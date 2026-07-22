#ifndef PLATO_MAIN_LIBRARY_EXITSTATUS
#define PLATO_MAIN_LIBRARY_EXITSTATUS

namespace plato::main::library
{
/// @brief enum representing the different exit conditions for plato.
enum struct ExitStatus : std::uint8_t
{
    kExecutionSuccessful = 0,
    kCommandLineInputError = 1,
    kInvalidInput = 2,
    kRuntimeError = 3
};
}  // namespace plato::main::library

#endif
