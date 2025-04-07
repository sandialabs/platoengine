#include "plato/process_manager/library/ProcessManagerRegistration.hpp"

#include "plato/core/InputVariantUtilities.hpp"

namespace plato::process_manager::library
{

[[nodiscard]] bool is_process_manager_function_registered(const std::string_view aFunctionName)
{
    return core::is_factory_function_registered<StageAndProcessManager, ValidatedProcessManagerInput>(aFunctionName);
}

auto is_new_process_manager_function_registered(const std::string_view aFunctionName) -> bool
{
    return core::is_factory_function_registered<StageAndProcessManager, NewValidatedProcessManagerInput>(aFunctionName);
}

}  // namespace plato::process_manager::library
