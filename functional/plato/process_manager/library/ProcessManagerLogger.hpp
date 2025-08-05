#ifndef PLATO_PROCESS_MANAGER_LIBRARY_PROCESSMANAGERLOGGER
#define PLATO_PROCESS_MANAGER_LIBRARY_PROCESSMANAGERLOGGER

#include "plato/input_parser/InputBlockStruct.hpp"
#include "plato/input_parser/InputBlockUtilities.hpp"
#include "plato/services/SystemLogger.hpp"

namespace plato::process_manager::library
{
/// @brief Creates a logger for process managers, using the input parser's block name.
template <input_parser::InputBlockOfComponent<components::ComponentType::kProcessManager> Input>
[[nodiscard]] auto process_manager_logger() -> services::SystemLogger
{
    return services::component_logger(components::ComponentType::kProcessManager, input_parser::block_name<Input>());
}

}  // namespace plato::process_manager::library

#endif
