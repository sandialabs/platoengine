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

/// @brief Creates a TaskLogger for the vector-Jacobian multiplication step of a filter component.
template <input_parser::InputBlockOfComponent<components::ComponentType::kFilter> Input>
[[nodiscard]] auto adjoint_jacobian_task_log() -> services::TaskLogSetupTeardown;

namespace detail
{
/// @brief Creates a logger for filter components, using the input parser's block name.
template <input_parser::InputBlockOfComponent<components::ComponentType::kFilter> Input>
[[nodiscard]] auto filter_logger() -> services::SystemLogger
{
    return services::component_logger(components::ComponentType::kFilter, input_parser::block_name<Input>());
}
}  // namespace detail

template <input_parser::InputBlockOfComponent<components::ComponentType::kFilter> Input>
auto filter_field_task_log() -> services::TaskLogSetupTeardown
{
    return services::TaskLogSetupTeardown{"Filtering field", detail::filter_logger<Input>()};
}

template <input_parser::InputBlockOfComponent<components::ComponentType::kFilter> Input>
auto jacobian_task_log() -> services::TaskLogSetupTeardown
{
    return services::TaskLogSetupTeardown{services::jacobian_task_message(), detail::filter_logger<Input>()};
}

/// @brief Creates a TaskLogger for the vector-Jacobian multiplication step of a filter component.
template <input_parser::InputBlockOfComponent<components::ComponentType::kFilter> Input>
auto adjoint_jacobian_task_log() -> services::TaskLogSetupTeardown
{
    return services::TaskLogSetupTeardown{services::adjoint_jacobian_task_message(), detail::filter_logger<Input>()};
}

}  // namespace plato::filter::library

#endif
