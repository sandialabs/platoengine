#include "plato/process_manager/library/ProcessManagerRegistration.hpp"

namespace plato::process_manager::library
{
auto is_new_process_manager_function_registered(const std::string_view aFunctionName) -> bool
{
    return core::is_factory_function_registered<StageAndProcessManager, NewValidatedProcessManagerInput>(aFunctionName);
}

}  // namespace plato::process_manager::library
