#ifndef PLATO_GEOMETRY_LIBRARY_GEOMETRYLOGGER
#define PLATO_GEOMETRY_LIBRARY_GEOMETRYLOGGER

#include "plato/input_parser/InputBlockStruct.hpp"
#include "plato/input_parser/InputBlockUtilities.hpp"
#include "plato/services/SystemLogger.hpp"
#include "plato/services/TaskLogSetupTeardown.hpp"

namespace plato::geometry::library
{
/// @brief Creates a TaskLogger for the mesh generation step of a geometry component.
template <input_parser::InputBlockOfComponent<components::ComponentType::kGeometry> Input>
[[nodiscard]] auto mesh_generation_task_log();

/// @brief Creates a TaskLogger for the output step of a geometry component.
template <input_parser::InputBlockOfComponent<components::ComponentType::kGeometry> Input>
[[nodiscard]] auto output_task_log();

/// @brief Creates a TaskLogger for the vector-Jacobian multiplication step of a geometry component.
template <input_parser::InputBlockOfComponent<components::ComponentType::kGeometry> Input>
[[nodiscard]] auto jacobian_task_log();

/// @brief Creates a TaskLogger for the vector-Jacobian multiplication step of a geometry component.
template <input_parser::InputBlockOfComponent<components::ComponentType::kGeometry> Input>
[[nodiscard]] auto adjoint_jacobian_task_log();

/// @brief Creates a logger for geometry components, using the input parser's block name.
template <input_parser::InputBlockOfComponent<components::ComponentType::kGeometry> Input>
[[nodiscard]] auto geometry_logger() -> services::SystemLogger;

namespace detail
{
/// @brief Creates a logger for geometry components, using the input parser's block name.
template <input_parser::InputBlockOfComponent<components::ComponentType::kGeometry> Input>
[[nodiscard]] auto geometry_task_logger(const std::string_view aTaskMessage) -> services::TaskLogSetupTeardown
{
    return services::TaskLogSetupTeardown{std::string{aTaskMessage}, geometry_logger<Input>()};
}
}  // namespace detail

template <input_parser::InputBlockOfComponent<components::ComponentType::kGeometry> Input>
[[nodiscard]] auto geometry_logger() -> services::SystemLogger
{
    return services::component_logger(components::ComponentType::kGeometry, input_parser::block_name<Input>());
}

template <input_parser::InputBlockOfComponent<components::ComponentType::kGeometry> Input>
[[nodiscard]] auto mesh_generation_task_log()
{
    return detail::geometry_task_logger<Input>("Generating mesh");
}

template <input_parser::InputBlockOfComponent<components::ComponentType::kGeometry> Input>
[[nodiscard]] auto output_task_log()
{
    return detail::geometry_task_logger<Input>("Writing output");
}

template <input_parser::InputBlockOfComponent<components::ComponentType::kGeometry> Input>
[[nodiscard]] auto jacobian_task_log()
{
    return detail::geometry_task_logger<Input>("Computing vector-Jacobian product");
}

template <input_parser::InputBlockOfComponent<components::ComponentType::kGeometry> Input>
[[nodiscard]] auto adjoint_jacobian_task_log()
{
    return detail::geometry_task_logger<Input>("Computing vector-adjoint-Jacobian product");
}

}  // namespace plato::geometry::library

#endif
