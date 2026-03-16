#ifndef PLATO_FILTER_LIBRARY_FILTERLOGGER
#define PLATO_FILTER_LIBRARY_FILTERLOGGER

#include "plato/input_parser/InputBlockStruct.hpp"
#include "plato/input_parser/InputBlockUtilities.hpp"
#include "plato/services/SystemLogger.hpp"
#include "plato/services/TaskLogSetupTeardown.hpp"

namespace plato::filter::library
{
/// @brief Creates a TaskLogger for the filter field step of a filter component.
template <input_parser::InputBlockOfComponent<components::ComponentType::kFilter> Input>
[[nodiscard]] auto filter_field_task_log() -> services::TaskLogSetupTeardown;

/// @brief Creates a TaskLogger for the vector-Jacobian multiplication step of a filter component.
template <input_parser::InputBlockOfComponent<components::ComponentType::kFilter> Input>
[[nodiscard]] auto jacobian_task_log() -> services::TaskLogSetupTeardown;

/// @brief Creates a TaskLogger for the vector-adjoint Jacobian multiplication step of a filter component.
template <input_parser::InputBlockOfComponent<components::ComponentType::kFilter> Input>
[[nodiscard]] auto adjoint_jacobian_task_log() -> services::TaskLogSetupTeardown;

/// @brief Creates a TaskLogger for the filter field step of a filter component.
[[nodiscard]] auto filter_field_task_log(std::string_view aFilterName) -> services::TaskLogSetupTeardown;

/// @brief Creates a TaskLogger for the vector-Jacobian multiplication step of a filter component.
[[nodiscard]] auto jacobian_task_log(std::string_view aFilterName) -> services::TaskLogSetupTeardown;

/// @brief Creates a TaskLogger for the vector-adjoint Jacobian multiplication step of a filter component.
[[nodiscard]] auto adjoint_jacobian_task_log(std::string_view aFilterName) -> services::TaskLogSetupTeardown;

namespace detail
{
/// @brief Creates a logger for filter components, using the input parser's block name.
template <input_parser::InputBlockOfComponent<components::ComponentType::kFilter> Input>
[[nodiscard]] auto filter_logger() -> services::SystemLogger
{
    return filter_logger(input_parser::block_name<Input>());
}

/// @brief Creates a logger for filter components, using @a aFilterName as the filter component's name in the log.
[[nodiscard]] auto filter_logger(const std::string_view aFilterName) -> services::SystemLogger;
}  // namespace detail

template <input_parser::InputBlockOfComponent<components::ComponentType::kFilter> Input>
auto filter_field_task_log() -> services::TaskLogSetupTeardown
{
    return filter_field_task_log(input_parser::block_name<Input>());
}

template <input_parser::InputBlockOfComponent<components::ComponentType::kFilter> Input>
auto jacobian_task_log() -> services::TaskLogSetupTeardown
{
    return jacobian_task_log(input_parser::block_name<Input>());
}

/// @brief Creates a TaskLogger for the vector-Jacobian multiplication step of a filter component.
template <input_parser::InputBlockOfComponent<components::ComponentType::kFilter> Input>
auto adjoint_jacobian_task_log() -> services::TaskLogSetupTeardown
{
    return adjoint_jacobian_task_log(input_parser::block_name<Input>());
}

}  // namespace plato::filter::library

#endif
