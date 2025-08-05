#ifndef PLATO_GEOMETRY_LIBRARY_GEOMETRYLOGGER
#define PLATO_GEOMETRY_LIBRARY_GEOMETRYLOGGER

#include "plato/input_parser/InputBlockStruct.hpp"
#include "plato/input_parser/InputBlockUtilities.hpp"
#include "plato/services/SystemLogger.hpp"

namespace plato::geometry::library
{
/// @brief Creates a logger for geometry components, using the input parser's block name.
template <input_parser::InputBlockOfComponent<components::ComponentType::kGeometry> Input>
[[nodiscard]] auto geometry_logger() -> services::SystemLogger
{
    return services::component_logger(components::ComponentType::kGeometry, input_parser::block_name<Input>());
}

}  // namespace plato::geometry::library

#endif
