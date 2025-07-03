#include "plato/process_manager/library/ProcessManagerValidation.hpp"

#include <optional>

#include "plato/input_validation/ValidationRegistration.hpp"
#include "plato/process_manager/library/ProcessManagerRegistration.hpp"
#include "plato/utilities/StringUtilities.hpp"

namespace plato::process_manager::library
{
[[maybe_unused]] static auto kProcessManagerValidationRegistration =
    input_validation::ParsedInputValidationRegistration<>{
        [](const input_parser::ParsedInput& aInput) { return detail::validate_at_least_one_process_manager(aInput); }};

namespace detail
{
auto validate_at_least_one_process_manager(const input_parser::ParsedInput& aInput) -> std::optional<std::string>
{
    if (aInput.get<components::ComponentType::kProcessManager>().empty())
    {
        const auto tAllProcessManagerNames =
            input_parser::all_block_names_with_component_type<components::ComponentType::kProcessManager>();
        return "At least one of the following blocks must be defined: " +
               utilities::concatenate_container(tAllProcessManagerNames, "\n");
    }
    return std::nullopt;
}
}  // namespace detail
}  // namespace plato::process_manager::library
