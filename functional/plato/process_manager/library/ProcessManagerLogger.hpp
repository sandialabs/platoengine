#ifndef PLATO_PROCESS_MANAGER_LIBRARY_PROCESSMANAGERLOGGER
#define PLATO_PROCESS_MANAGER_LIBRARY_PROCESSMANAGERLOGGER

#include "plato/input_parser/InputBlockStruct.hpp"
#include "plato/input_parser/InputBlockUtilities.hpp"
#include "plato/services/SystemLogger.hpp"
#include "plato/services/TaskLogSetupTeardown.hpp"

namespace plato::process_manager::library
{
/// @brief Creates a TaskLogger for the mesh generation step of a geometry component.
template <input_parser::InputBlockOfComponent<components::ComponentType::kProcessManager> Input>
[[nodiscard]] auto run_task_log() -> services::TaskLogSetupTeardown;

/// @brief Creates a logger for process managers, using the input parser's block name.
template <input_parser::InputBlockOfComponent<components::ComponentType::kProcessManager> Input>
[[nodiscard]] auto process_manager_logger() -> services::SystemLogger;

template <input_parser::InputBlockOfComponent<components::ComponentType::kProcessManager> Input>
auto run_task_log() -> services::TaskLogSetupTeardown
{
    return services::TaskLogSetupTeardown{"Running", process_manager_logger<Input>()};
}

template <input_parser::InputBlockOfComponent<components::ComponentType::kProcessManager> Input>
[[nodiscard]] auto process_manager_logger() -> services::SystemLogger
{
    return services::component_logger(components::ComponentType::kProcessManager, input_parser::block_name<Input>());
}

}  // namespace plato::process_manager::library

#endif
