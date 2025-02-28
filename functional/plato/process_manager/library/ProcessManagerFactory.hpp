#ifndef PLATO_PROCESSMANAGER_LIBRARY_PROCESSMANAGERFACTORY
#define PLATO_PROCESSMANAGER_LIBRARY_PROCESSMANAGERFACTORY

#include <string>
#include <vector>

#include "plato/process_manager/library/ProcessManagerRegistration.hpp"

namespace plato::process_manager::library
{
/// @brief Creates all ProcessManager objects from the parsed and validated input in
///  @a aValidatedProcessManagerInput.
///
/// @post The order of the objects in the returned is guaranteed to be sorted based on
///  each object's RunStage provided to the factory. Within each RunStage, no order
///  is guaranteed.
[[nodiscard]] std::vector<ProcessManager> make_process_managers(
    const ValidatedProcessManagerInputVector& aValidatedProcessManagerInput);

/// @brief Returns a list of process managers defined in the input @a aValidatedProcessManagerInput, but not registered
/// with the factory.
///
/// This can happen for process managers such as SNOPT that are conditionally built. Their input block definitions will
/// be built, but they will not be statically registered.
[[nodiscard]] auto unregistered_process_managers(
    const ValidatedProcessManagerInputVector& aValidatedProcessManagerInput) -> std::vector<std::string>;

}  // namespace plato::process_manager::library

#endif
