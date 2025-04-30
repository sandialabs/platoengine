#include "plato/process_manager/library/ProcessManagerRegistration.hpp"

namespace plato::process_manager::library
{
auto is_process_manager_function_registered(const std::string_view aFunctionName) -> bool
{
    return core::is_factory_function_registered<StageAndProcessManager, ValidatedProcessManagerInput>(aFunctionName);
}

}  // namespace plato::process_manager::library
