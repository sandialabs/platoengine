#ifndef PLATO_PROCESSMANAGER_LIBRARY_PROCESSMANAGERVALIDATION
#define PLATO_PROCESSMANAGER_LIBRARY_PROCESSMANAGERVALIDATION

#include <optional>
#include <string>

#include "plato/input_parser/ParsedInput.hpp"

namespace plato::process_manager::library::detail
{
/// @brief Checks that at least one process manager appears in the input. If not, an error message is returned.
[[nodiscard]] auto validate_at_least_one_process_manager(const input_parser::NewParsedInput& aInput)
    -> std::optional<std::string>;
}  // namespace plato::process_manager::library::detail

#endif
