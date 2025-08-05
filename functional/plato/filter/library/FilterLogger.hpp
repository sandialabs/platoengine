#ifndef PLATO_FILTER_LIBRARY_FILTERLOGGER
#define PLATO_FILTER_LIBRARY_FILTERLOGGER

#include "plato/input_parser/InputBlockStruct.hpp"
#include "plato/input_parser/InputBlockUtilities.hpp"
#include "plato/services/SystemLogger.hpp"

namespace plato::filter::library
{
/// @brief Creates a logger for filter components, using the input parser's block name.
template <input_parser::InputBlockOfComponent<components::ComponentType::kFilter> Input>
[[nodiscard]] auto filter_logger() -> services::SystemLogger
{
    return services::component_logger(components::ComponentType::kFilter, input_parser::block_name<Input>());
}

}  // namespace plato::filter::library

#endif
